#ifndef PA_SINK_H
#define PA_SINK_H

extern "C" {
#include <config.h>
#include <pulse/def.h>
#include <pulsecore/module.h>
#include <pulsecore/thread.h>
} // extern "C"

#include <QtCore/QScopedPointer>
#include <QtCore/QtGlobal>

class Writer;

class PASink {
public:
    int init(pa_module *m, Writer *writer);
    static PASink &instance();
    void drop();

    int sampleRateHz() const;
    int bitRateBps() const;
    int numChannels() const;

    int onSinkProcessMsg(pa_msgobject *o, int code, void *data, int64_t offset,
                         pa_memchunk *chunk);
    void onSinkUpdateRequestedLatency(pa_sink *s);
    void threadFunc();

private:
    void processRender(pa_usec_t now);
    void processRewind(pa_usec_t now);


    friend class QScopedPointerDeleter<PASink>;
    PASink();
    virtual ~PASink();
    Q_DISABLE_COPY(PASink)

    static QScopedPointer<PASink> m_instance;

    pa_module *m_module;
    Writer *m_writer;

    pa_sink *m_sink;

    pa_thread *m_thread;
    pa_thread_mq m_thread_mq;
    pa_rtpoll *m_rtpoll;

    pa_usec_t m_block_usec;
    pa_usec_t m_timestamp;

    pa_subscription *m_event_subscription;
};

#endif // PA_SINK_H
