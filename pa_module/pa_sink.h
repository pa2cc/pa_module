#ifndef PA_SINK_H
#define PA_SINK_H

extern "C" {
#include <config.h>
#include <pulse/def.h>
#include <pulsecore/module.h>
#include <pulsecore/thread.h>
} // extern "C"

#include <QScopedPointer>
#include <QtGlobal>

template<class T> class ChangeNotifier;
class Writer;

class PASink {
public:
    int init(pa_module *m, Writer *writer);
    static PASink &instance();
    void drop();

    ChangeNotifier<int> *volume_notifier() const;

    int sink_process_msg(pa_msgobject *o, int code, void *data, int64_t offset,
                         pa_memchunk *chunk);
    void sink_update_requested_latency(pa_sink *s);
    void sink_input_event(pa_subscription_event_type_t event_type,
                          uint32_t idx);
    void thread_func();

private:
    void process_render(pa_usec_t now);
    void process_rewind(pa_usec_t now);
    void update_volume(bool force_update);


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
    QScopedPointer<ChangeNotifier<int>> m_volume_notifier;
};

#endif // PA_SINK_H
