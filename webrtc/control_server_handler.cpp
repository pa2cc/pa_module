#include "control_server_handler.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonValue>

#include "conductor.h"
#include "control_server.h"

namespace {
const int kControlServerPort = 51348;

const QString kMsgTypeReset = "reset";
const QString kMsgTypeGetIceCandidates = "getIceCandidates";
const QString kMsgTypeGetSessionDescription = "getSessionDescription";

// Names used for a IceCandidate JSON object.
const QString kMsgTypeIceCandidate = "iceCandidate";
const QString kCandidateSdpMidName = "sdpMid";
const QString kCandidateSdpMlineIndexName = "sdpMLineIndex";
const QString kCandidateSdpName = "candidate";

// Names used for a SessionDescription JSON object.
const QString kMsgTypeSessionDescription = "sessionDescription";
const QString kSessionDescriptionTypeName = "type";
const QString kSessionDescriptionSdpName = "sdp";

} // namespace

namespace webrtc {

ControlServerHandler::ControlServerHandler(ControlServer *control_server,
                                           Conductor *conductor)
    : m_control_server(control_server)
    , m_conductor(conductor)
{
    connect(control_server, &ControlServer::clientConnected,
            this, &ControlServerHandler::onClientConnected);
    connect(control_server, &ControlServer::messageReceived,
            this, &ControlServerHandler::onMessage);
}

ControlServerHandler::~ControlServerHandler() {
}

void ControlServerHandler::onMessage(const QString &type,
                                     const QJsonValue &payload) {
    if (kMsgTypeReset == type) {
        handleReset();
    } else if (kMsgTypeGetIceCandidates == type) {
        handleGetIceCandidates();
    } else if (kMsgTypeGetSessionDescription == type) {
        handleGetSessionDescription();
    } else if (kMsgTypeIceCandidate == type) {
        handleIceCandidate(payload);
    } else if (kMsgTypeSessionDescription == type) {
        handleSessionDescription(payload);
    }
}

void ControlServerHandler::onClientConnected() {
    Q_ASSERT(m_conductor->open() &&
             "Failed to initialize the conductor.");
}

void ControlServerHandler::handleReset() {
    m_conductor->close();
    Q_ASSERT(m_conductor->open() &&
             "Failed to initialize the conductor.");
}


void ControlServerHandler::sendIceCandidate(
        const IceCandidateInterface *ice_candidate) {
    // Constructs the json candidate.
    std::string sdp;
    Q_ASSERT(ice_candidate->ToString(&sdp) &&
             "Failed to serialize candidate");

    QJsonObject json_candidate;
    json_candidate[kCandidateSdpMidName] =
            QString::fromStdString(ice_candidate->sdp_mid());
    json_candidate[kCandidateSdpMlineIndexName] =
            ice_candidate->sdp_mline_index();
    json_candidate[kCandidateSdpName] = QString::fromStdString(sdp);

    m_control_server->sendMessage(kMsgTypeIceCandidate, json_candidate);
}

void ControlServerHandler::sendSessionDescription(
        const SessionDescriptionInterface *session_description) {
    std::string sdp;
    Q_ASSERT(session_description->ToString(&sdp) &&
             "Failed to serialize session description");

    QJsonObject json_session_description;
    json_session_description[kSessionDescriptionTypeName] =
            QString::fromStdString(session_description->type());
    json_session_description[kSessionDescriptionSdpName] =
            QString::fromStdString(sdp);

    m_control_server->sendMessage(kMsgTypeSessionDescription,
                                  json_session_description);
}


void ControlServerHandler::handleGetIceCandidates() {
    // Sends the already known ice candidates. New ones are forwarded as they
    // appear.
    foreach (const IceCandidateInterface *candidate,
             m_conductor->iceCandidates()) {
        sendIceCandidate(candidate);
    }

    // Subscribes for further appearing candidates.
    connect(m_conductor, &Conductor::iceCandidateAppeared,
            [=](const IceCandidateInterface *candidate) {
        sendIceCandidate(candidate);
    });
}

void ControlServerHandler::handleGetSessionDescription() {
    // If there already is a session description around we immediately send it.
    // Otherwise it will be forwarded as it appears.
    const SessionDescriptionInterface *local_description =
            m_conductor->localDescription();
    if (local_description) {
        sendSessionDescription(local_description);
    }

    // Subscribes for further appearing descriptions.
    connect(m_conductor, &Conductor::localDescriptionAppeared,
            [=](const SessionDescriptionInterface *session_description) {
        sendSessionDescription(session_description);
    });
}

void ControlServerHandler::handleIceCandidate(const QJsonValue &payload) {
    if (!payload.isObject()) {
        qWarning() << "Invalid ice candidate payload";
        return;
    }

    // Parses the session description.
    QJsonObject json_candidate = payload.toObject();
    QString sdp_mid = json_candidate[kCandidateSdpMidName].toString();
    int sdp_mlineindex = json_candidate[kCandidateSdpMlineIndexName].toInt();
    QString sdp = json_candidate[kCandidateSdpName].toString();

    rtc::scoped_ptr<webrtc::IceCandidateInterface> ice_candidate(
                webrtc::CreateIceCandidate(sdp_mid.toStdString(),
                                           sdp_mlineindex, sdp.toStdString(),
                                           NULL));
    if (!ice_candidate.get()) {
        qWarning() << "Invalid ice candidate payload";
        return;
    }

    // Sets the remote session description (does not take ownership).
    m_conductor->addRemoteIceCandidate(ice_candidate.get());
}

void ControlServerHandler::handleSessionDescription(const QJsonValue &payload) {
    if (!payload.isObject()) {
        qWarning() << "Invalid session description payload";
        return;
    }

    // Parses the session description.
    QJsonObject json_session_description = payload.toObject();
    QString type =
            json_session_description[kSessionDescriptionTypeName].toString();
    QString sdp =
            json_session_description[kSessionDescriptionSdpName].toString();

    webrtc::SessionDescriptionInterface *session_description(
                webrtc::CreateSessionDescription(type.toStdString(),
                                                 sdp.toStdString(),
                                                 NULL));
    if (!session_description) {
        qWarning() << "Invalid session description payload";
        return;
    }

    // Sets the remote session description.
    m_conductor->setRemoteDescription(session_description); // Takes ownership.
}

} // namespace webrtc
