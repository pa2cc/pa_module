#ifndef WEBRTC_WRITER_WEBRTC_PRIV_H
#define WEBRTC_WRITER_WEBRTC_PRIV_H

#include <QtCore/QObject>

class ControlServer;

namespace webrtc {
class AudioDeviceModule;
} // namespace webrtc

class Worker : public QObject {
    Q_OBJECT

public:
    Worker(ControlServer *control_server, webrtc::AudioDeviceModule *adm)
        : m_control_server(control_server)
        , m_adm(adm)
    {}
    ~Worker();

public Q_SLOTS:
    void run();

private:
    ControlServer *m_control_server;
    webrtc::AudioDeviceModule *m_adm;
};


#endif // WEBRTC_WRITER_WEBRTC_PRIV_H

