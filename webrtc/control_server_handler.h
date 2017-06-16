#ifndef WEBRTC_CONTROL_SERVER_HANDLER_H
#define WEBRTC_CONTROL_SERVER_HANDLER_H

#include <QtCore/QObject>

class ControlServer;
class QJsonValue;

namespace webrtc {

class Conductor;
class IceCandidateInterface;
class SessionDescriptionInterface;

class ControlServerHandler : public QObject {
    Q_OBJECT

public:
    ControlServerHandler(ControlServer *control_server, Conductor *conductor);
    ~ControlServerHandler();

private Q_SLOTS:
    void onClientConnected();
    void onMessage(const QString &type, const QJsonValue &payload);

private:
    void sendIceCandidate(const IceCandidateInterface *ice_candidate);
    void sendSessionDescription(
            const SessionDescriptionInterface *session_description);

    void handleReset();
    void handleGetIceCandidates();
    void handleGetSessionDescription();
    void handleSessionDescription(const QJsonValue &payload);
    void handleIceCandidate(const QJsonValue &payload);

    ControlServer *m_control_server;
    Conductor *m_conductor;
};

} // namespace webrtc

#endif // WEBRTC_CONTROL_SERVER_HANDLER_H
