#include "writer_webrtc.h"
#include "writer_webrtc_priv.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <webrtc/base/scoped_ref_ptr.h>
#include <webrtc/base/ssladapter.h>
#include <webrtc/base/thread.h>
#pragma GCC diagnostic pop

#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

#include "conductor.h"
#include "control_server.h"
#include "control_server_handler.h"
#include "pa_audio_device_module.h"

namespace {
const pa_sample_format_t kSampleFormat = PA_SAMPLE_S16LE;
} // namespace

class WebRTCWriterPriv {
public:
    QThread thread;
    rtc::scoped_refptr<webrtc::PAAudioDeviceModule> adm;
    QScopedPointer<Worker> worker;
};

WebRTCWriter::WebRTCWriter(PASink *pa_sink, ControlServer *control_server)
    : BaseWriter(pa_sink)
    , d(new WebRTCWriterPriv)
{
    d->adm = new rtc::RefCountedObject<webrtc::PAAudioDeviceModule>(pa_sink);
    d->worker.reset(new Worker(control_server, d->adm.get()));

    d->worker->moveToThread(&d->thread);
    QMetaObject::invokeMethod(d->worker.data(), "run", Qt::QueuedConnection);

    d->thread.start();
}

WebRTCWriter::~WebRTCWriter() {
    d->thread.quit();
    d->thread.wait();
}

pa_sample_format_t WebRTCWriter::sampleFormat() const {
    return kSampleFormat;
}

ssize_t WebRTCWriter::write(const void *buf, size_t length) {
    return d->adm->write(buf, length);
}

/********************************* WORKER *************************************/

Worker::~Worker() {
}

void Worker::run() {
    // Runs in own thread.

    // Initializes the rtc thread.
    rtc::AutoThread auto_thread;
    rtc::Thread *thread = rtc::Thread::Current();

    rtc::InitializeSSL();

    // Initializes the conductor.
    rtc::scoped_refptr<webrtc::Conductor> conductor(
                new rtc::RefCountedObject<webrtc::Conductor>(m_adm));

    // Initializes the control-server handler.
    webrtc::ControlServerHandler control_server_handler(m_control_server,
                                                        conductor.get());

    for (;;) {
        // Processes rtc events.
        bool ok = thread->ProcessMessages(100);
        if (!ok) {
            break;
        }

        // Processes Qt events.
        QCoreApplication::processEvents();
    }

    rtc::CleanupSSL();
}
