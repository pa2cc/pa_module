#ifndef PA_MODULE_H
#define PA_MODULE_H

extern "C" {
#include <config.h>
#include <pulsecore/module.h>
#include <pulsecore/thread.h>
} // extern "C"

#include <QtCore/QCoreApplication>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QThread>

class ControlServer;
class Writer;

class PAModule : public QObject {
    Q_OBJECT

public:
    int init(pa_module *m);
    static PAModule &instance();
    void stop();

private Q_SLOTS:
    void exec();

private:
    friend class QScopedPointerDeleter<PAModule>;

    PAModule();
    virtual ~PAModule();
    Q_DISABLE_COPY(PAModule)

    static QScopedPointer<PAModule> m_instance;

    QCoreApplication *m_application;

    pa_module *m_module;

    QThread m_main_thread;

    QScopedPointer<ControlServer> m_control_server;
    QScopedPointer<Writer> m_writer;
};
#endif // PA_MODULE_H

