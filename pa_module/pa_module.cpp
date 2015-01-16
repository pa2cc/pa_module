#include "pa_module.h"

#include <QDebug>
#include <QThread>

#include "control_server.h"
#include "pa_sink.h"
#include "streaming_server.h"
#include "writer_hls.h"

PA_MODULE_AUTHOR("Sämy Zehnder");
PA_MODULE_DESCRIPTION("PACC");
PA_MODULE_VERSION(LIBRARY_VERSION);
PA_MODULE_LOAD_ONCE(true);

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

    // Initializes the writer.
    m_writer.reset(new HLSWriter);

    // Initializes the sink.
    PASink::instance().init(m_module, m_writer.data());

    // Creates the main thread.
    connect(&m_main_thread, SIGNAL(started()), this, SLOT(exec()),
            Qt::DirectConnection);
    m_main_thread.start();

    return 0;
}

void PAModule::exec() {
    pa_log_debug("Main thread starting up");

    // Initializes the QCoreApplication.
    Q_ASSERT(!QCoreApplication::instance());
    int argc = 0;
    QCoreApplication app(argc, NULL);
    m_application = &app;

    // Starts the servers.
    const QString &stream_secret = StreamingServer::generateStreamSecret();
    m_control_server.reset(new ControlServer(stream_secret));
    m_streaming_server.reset(new StreamingServer(stream_secret));

    app.exec();

    m_application = NULL;

    m_main_thread.quit();
    m_instance.reset(); // Self destruction (No problem if already destructing).
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
    m_streaming_server.reset();
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
