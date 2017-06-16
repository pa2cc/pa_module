#include "pa_sink.h"
#include "pa_sink_priv.h"

extern "C" {
#include <pulse/introspect.h>
#include <pulse/rtclock.h>
#include <pulse/timeval.h>
#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/namereg.h>
} // extern "C"

#include "writer.h"

namespace {
const int kSampleRateHz = 44100;
const int kBitRateBps   = 128000;
const int kNumChannels  = 2;

const char kDefaultSinkName[] = "PACC bridge";
const pa_usec_t kBlocUsec = 2 * PA_USEC_PER_SEC;

const char* const kValidModargs[] = {
    "sink_name",
    "sink_properties",
    "master",
    "rate",
    "channels",
    "channel_map",
    NULL
};
} // namespace

static void sinkEventCb(pa_core *core, pa_subscription_event_type_t event_type,
                        uint32_t idx, void *self);
static int sinkProcessMsgCb(pa_msgobject *o, int code, void *data,
                            int64_t offset, pa_memchunk *chunk);
static void sinkUpdateRequestedLatencyCb(pa_sink *s);
static void threadFuncCb(void *self);


PASink::PASink()
    : d(new PASinkPriv)
{
}

PASink::~PASink() {
}

QScopedPointer<PASink> PASink::m_instance(NULL);
PASink &PASink::instance() {
    if (!m_instance) {
        m_instance.reset(new PASink);
    }

    return *m_instance;
}
void PASink::drop() {
    m_instance.reset(NULL);
}


int PASink::init(pa_module *module, Writer *writer) {
    return d->init(module, writer);
}

int PASink::sampleRateHz() const {
    return kSampleRateHz;
}
int PASink::bitRateBps() const {
    return kBitRateBps;
}
int PASink::numChannels() const {
    return kNumChannels;
}

bool PASink::isMuted() const {
    return d->m_muted;
}

quint32 PASink::volume() const {
    return d->m_avg_volume;
}
quint32 PASink::minVolume() const {
    return PA_VOLUME_MUTED;
}
quint32 PASink::maxVolume() const {
    return PA_VOLUME_NORM;
}
quint16 PASink::volumeStepSize() const {
    return (PA_VOLUME_NORM + 1) / d->m_sink->n_volume_steps;
}

/********************************** PRIV **************************************/

PASinkPriv::PASinkPriv()
    : m_module(NULL)
    , m_writer(NULL)

    , m_sink(NULL)
    , m_thread(NULL)
    , m_rtpoll(NULL)

    , m_event_subscription(NULL)

    , m_muted(false)
    , m_avg_volume(0)
{
}

int PASinkPriv::init(pa_module *m, Writer *writer) {
    pa_assert_se(m_module = m);
    pa_assert_se(m_writer = writer);

    pa_modargs *ma = pa_modargs_new(m_module->argument, kValidModargs);
    if (!ma) {
        pa_log("Failed to parse module arguments.");
        goto fail;
    }

    pa_channel_map map;
    switch (kNumChannels) {
        case 1: pa_channel_map_init_mono(&map); break;
        case 2: pa_channel_map_init_stereo(&map); break;

        default: Q_ASSERT(false);
    }

    // Initializes the sample specs.
    pa_sample_spec ss;
    pa_sample_spec_init(&ss);
    ss.format = m_writer->sampleFormat();
    ss.rate = kSampleRateHz;
    ss.channels = map.channels;
    pa_assert(pa_sample_spec_valid(&ss));

    m_rtpoll = pa_rtpoll_new();
    pa_thread_mq_init(&m_thread_mq, m_module->core->mainloop, m_rtpoll);

    // Creates sink.
    pa_sink_new_data sink_data;
    pa_sink_new_data_init(&sink_data);
    sink_data.driver = __FILE__;
    sink_data.module = m_module;
    pa_sink_new_data_set_name(
                &sink_data,
                pa_modargs_get_value(ma, "sink_name", kDefaultSinkName));
    pa_proplist_setf(sink_data.proplist, PA_PROP_DEVICE_DESCRIPTION, "PACC");
    pa_sink_new_data_set_sample_spec(&sink_data, &ss);
    pa_sink_new_data_set_channel_map(&sink_data, &map);

    if (pa_modargs_get_proplist(ma, "sink_properties", sink_data.proplist,
                                PA_UPDATE_REPLACE) < 0) {
        pa_log("Invalid properties");
        pa_sink_new_data_done(&sink_data);
        goto fail;
    }

    m_sink = pa_sink_new(
                m_module->core, &sink_data,
                (pa_sink_flags_t)(PA_SINK_LATENCY|PA_SINK_DYNAMIC_LATENCY));
    pa_sink_new_data_done(&sink_data);

    if (!m_sink) {
        pa_log("Failed to create sink.");
        goto fail;
    }

    m_sink->parent.process_msg = sinkProcessMsgCb;
    m_sink->update_requested_latency = sinkUpdateRequestedLatencyCb;

    pa_sink_set_asyncmsgq(m_sink, m_thread_mq.inq);
    pa_sink_set_rtpoll(m_sink, m_rtpoll);

    m_block_usec = kBlocUsec;
    size_t nbytes;
    nbytes = pa_usec_to_bytes(m_block_usec, &m_sink->sample_spec);
    pa_sink_set_max_rewind(m_sink, nbytes);
    pa_sink_set_max_request(m_sink, nbytes);

    if (!(m_thread = pa_thread_new("pacc-sink", threadFuncCb, this))) {
        pa_log("Failed to create thread.");
        goto fail;
    }

    pa_sink_put(m_sink);

    // Subscribes us to events on the sink.
    m_event_subscription = pa_subscription_new(m_module->core,
                                               PA_SUBSCRIPTION_MASK_SINK,
                                               sinkEventCb, this);

    pa_modargs_free(ma);

    return 0;

fail:
    if (ma) {
        pa_modargs_free(ma);
    }
    return -1;
}

PASinkPriv::~PASinkPriv() {
    if (!m_module) {
        return;
    }

    if (m_event_subscription) {
        pa_subscription_free(m_event_subscription);
    }

    if (m_sink) {
        pa_sink_unlink(m_sink);
    }

    if (m_thread) {
        pa_asyncmsgq_send(m_thread_mq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0,
                          NULL);
        pa_thread_free(m_thread);
    }

    pa_thread_mq_done(&m_thread_mq);

    if (m_sink) {
        pa_sink_unref(m_sink);
    }

    if (m_rtpoll) {
        pa_rtpoll_free(m_rtpoll);
    }
}

void sinkEventCb(pa_core *core, pa_subscription_event_type_t event_type,
                 uint32_t idx, void *self) {
    Q_UNUSED(core);
    ((PASinkPriv *)self)->onSinkEvent(event_type, idx);
}
void PASinkPriv::onSinkEvent(pa_subscription_event_type_t event_type,
                             uint32_t idx) {
    if (m_sink->index != idx || event_type != PA_SUBSCRIPTION_EVENT_CHANGE) {
        return;
    }

    // Checks if we are muted and reads the volume.
    m_muted = pa_sink_get_mute(m_sink, false);
    const pa_cvolume *volumes = pa_sink_get_volume(m_sink, false);
    m_avg_volume = pa_cvolume_avg(volumes);
}

int sinkProcessMsgCb(pa_msgobject *o, int code, void *data, int64_t offset,
                     pa_memchunk *chunk) {
    return PASink::instance().d->onSinkProcessMsg(o, code, data, offset, chunk);
}
int PASinkPriv::onSinkProcessMsg(pa_msgobject *o, int code, void *data,
                                 int64_t offset, pa_memchunk *chunk) {
    switch (code) {
        case PA_SINK_MESSAGE_GET_LATENCY: {
            pa_usec_t now = pa_rtclock_now();
            *((pa_usec_t *)data) = m_timestamp > now ? m_timestamp - now : 0;
            return 0;
        }
    }

    return pa_sink_process_msg(o, code, data, offset, chunk);
}

void sinkUpdateRequestedLatencyCb(pa_sink *s) {
    PASink::instance().d->onSinkUpdateRequestedLatency(s);
}
void PASinkPriv::onSinkUpdateRequestedLatency(pa_sink *s) {
    pa_sink_assert_ref(s);

    m_block_usec = pa_sink_get_requested_latency_within_thread(s);

    if (m_block_usec == (pa_usec_t)-1) {
        m_block_usec = s->thread_info.max_latency;
    }

    size_t nbytes = pa_usec_to_bytes(m_block_usec, &s->sample_spec);
    pa_sink_set_max_rewind_within_thread(s, nbytes);
    pa_sink_set_max_request_within_thread(s, nbytes);
}

void PASinkPriv::processRewind(pa_usec_t now) {
    size_t rewind_nbytes = m_sink->thread_info.rewind_nbytes;

    if (!PA_SINK_IS_OPENED(m_sink->thread_info.state) || rewind_nbytes <= 0 ||
            m_timestamp <= now) {
        goto do_nothing;
    }

    pa_usec_t delay;
    delay = m_timestamp - now;

    size_t in_buffer;
    in_buffer = pa_usec_to_bytes(delay, &m_sink->sample_spec);
    if (in_buffer <= 0) {
        goto do_nothing;
    }

    if (rewind_nbytes > in_buffer) {
        rewind_nbytes = in_buffer;
    }

    pa_sink_process_rewind(m_sink, rewind_nbytes);
    m_timestamp -= pa_bytes_to_usec(rewind_nbytes, &m_sink->sample_spec);

    return;

do_nothing:
    pa_sink_process_rewind(m_sink, 0);
}

void PASinkPriv::processRender(pa_usec_t now) {
    /* This is the configured latency. Sink inputs connected to us
    might not have a single frame more than the maxrequest value
    queued. Hence: at maximum read this many bytes from the sink
    inputs. */

    /* Fill the buffer up the latency size */
    size_t ate = 0;
    while (m_timestamp < now + m_block_usec) {
        size_t max_request = m_sink->thread_info.max_request - ate;
        if (max_request <= 0) {
            break;
        }

        pa_memchunk chunk;
        pa_sink_render(m_sink, max_request, &chunk);

        void *p = pa_memblock_acquire(chunk.memblock);
        size_t bytes_written =
                m_writer->write((const uint8_t *)p + chunk.index, chunk.length);
        Q_ASSERT(bytes_written == chunk.length);
        pa_memblock_release(chunk.memblock);

        pa_memblock_unref(chunk.memblock);

        m_timestamp += pa_bytes_to_usec(chunk.length, &m_sink->sample_spec);

        ate += chunk.length;
    }
}

void threadFuncCb(void *self) {
    ((PASinkPriv *)self)->threadFunc();
}
void PASinkPriv::threadFunc() {
    pa_log_debug("Sink thread starting up");

    pa_thread_mq_install(&m_thread_mq);

    m_timestamp = pa_rtclock_now();
    for (;;) {
        pa_usec_t now = 0;

        if (PA_SINK_IS_OPENED(m_sink->thread_info.state)) {
            now = pa_rtclock_now();
        }

        if (PA_UNLIKELY(m_sink->thread_info.rewind_requested)) {
            processRewind(now);
        }

        /* Render some data and write it to the fifo */
        if (PA_SINK_IS_OPENED(m_sink->thread_info.state)) {
            if (m_timestamp <= now) {
                processRender(now);
            }

            pa_rtpoll_set_timer_absolute(m_rtpoll, m_timestamp);
        } else {
            pa_rtpoll_set_timer_disabled(m_rtpoll);
        }

        /* Hmm, nothing to do. Let's sleep */
        int ret = pa_rtpoll_run(m_rtpoll);
        if (ret < 0) {
            goto fail;
        } else if(ret == 0) {
            goto finish;
        }
    }

fail:
    /* If this was no regular exit from the loop we have to continue
     * processing messages until we received PA_MESSAGE_SHUTDOWN */
    pa_asyncmsgq_post(m_thread_mq.outq, PA_MSGOBJECT(m_module->core),
                      PA_CORE_MESSAGE_UNLOAD_MODULE, m_module, 0, NULL, NULL);
    pa_asyncmsgq_wait_for(m_thread_mq.inq, PA_MESSAGE_SHUTDOWN);

finish:
    pa_log_debug("Sink thread shutting down");
}
