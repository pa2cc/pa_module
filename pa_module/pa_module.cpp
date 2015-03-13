#include "pa_module.h"

extern "C" {
#include <config.h>
#include <pulsecore/module.h>
#include <pulsecore/thread.h>
} // extern "C"

#include "pa_sink.h"
#include "websocket_server.h"
#include "writer_webrtc.h"

PA_MODULE_AUTHOR("Sämy Zehnder");
PA_MODULE_DESCRIPTION("PACC");
PA_MODULE_VERSION(LIBRARY_VERSION);
PA_MODULE_LOAD_ONCE(true);

namespace {
const int kControlServerPort = 51348;
} // namespace

extern "C" {

int pa__init(pa_module *m) {
    // Starts the module up.
    return PAModule::instance().init(m);
}

void pa__done(pa_module *m) {
    // Shuts the module down.
    Q_UNUSED(m);
    PAModule::instance().stop();
}

} // extern "C"


PAModule::PAModule()
    : m_application(NULL)
{
}

int PAModule::init(pa_module *m) {
    pa_assert_se(m_module = m);

    connect(&m_main_thread, &QThread::started, this, &PAModule::exec,
            Qt::DirectConnection);

    // Self destruction (No problem if already destructing).
    connect(&m_main_thread, &QThread::finished, []() {
        PAModule *self = m_instance.take();
        if (self) {
            self->deleteLater();
        }
    });

    // Starts the main thread. All initialization is done on it.
    m_main_thread.start();

    return 0;
}

void PAModule::exec() {
    Q_ASSERT(QThread::currentThread() == &m_main_thread);
    pa_log_debug("Main thread starting up");

    // Initializes the QCoreApplication.
    Q_ASSERT(!QCoreApplication::instance());
    int argc = 0;
    QCoreApplication app(argc, NULL);

    // Initializes the control server.
    m_control_server.reset(new WebsocketServer(kControlServerPort));

    // Creates the sink.
    PASink *pa_sink = &PASink::instance();

    // Initializes the writer.
    m_writer.reset(new WebRTCWriter(pa_sink, m_control_server.data()));

    // Initializes the sink.
    pa_sink->init(m_module, m_writer.data());

    // Starts the QCoreApplication.
    m_application = &app;
    m_application->exec();
    m_application = NULL;

    // Tears down the thread.
    m_main_thread.quit();
}

void PAModule::stop() {
    m_instance.reset();
}

PAModule::~PAModule() {
    if (!m_module) {
        return;
    }

    if (m_application) {
        m_application->quit();
        m_main_thread.wait();
    }

    // Stops the servers.
    m_control_server.reset();

    // Stops the sink.
    PASink::instance().drop();
}

QScopedPointer<PAModule> PAModule::m_instance(NULL);
PAModule &PAModule::instance() {
    if (!m_instance) {
        m_instance.reset(new PAModule);
    }

    return *m_instance;
}
