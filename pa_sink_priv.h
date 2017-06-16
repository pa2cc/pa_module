#ifndef PA_SINK_PRIV_H
#define PA_SINK_PRIV_H

extern "C" {
#include <config.h>
#include <pulse/def.h>
#include <pulsecore/module.h>
#include <pulsecore/thread.h>
} // extern "C"

class PASink;
class Writer;

class PASinkPriv {
public:
    PASinkPriv();
    ~PASinkPriv();

    int init(pa_module *module, Writer *writer);

    void setMute(bool muted);
    void setVolume(pa_volume_t volume);

    void onSinkEvent(pa_subscription_event_type_t event_type, uint32_t idx);
    int onSinkProcessMsg(pa_msgobject *o, int code, void *data, int64_t offset,
                         pa_memchunk *chunk);
    void onSinkUpdateRequestedLatency(pa_sink *s);
    void threadFunc();

private:
    friend class PASink;
    void processRender(pa_usec_t now);
    void processRewind(pa_usec_t now);


    pa_module *m_module;
    Writer *m_writer;

    pa_sink *m_sink;

    pa_thread *m_thread;
    pa_thread_mq m_thread_mq;
    pa_rtpoll *m_rtpoll;

    pa_usec_t m_block_usec;
    pa_usec_t m_timestamp;

    pa_subscription *m_event_subscription;

    bool m_muted;
    pa_volume_t m_avg_volume;
};

#endif // PA_SINK_PRIV_H

