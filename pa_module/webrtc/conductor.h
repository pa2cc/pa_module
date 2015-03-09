#ifndef WEBRTC_CONDUCTOR_H
#define WEBRTC_CONDUCTOR_H

#include <string>

#include <talk/app/webrtc/mediastreaminterface.h>
#include <talk/app/webrtc/peerconnectioninterface.h>
#include <webrtc/base/scoped_ptr.h>

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QScopedPointer>
#include <QtCore/QString>

namespace rtc {
class Thread;
} // namespace rtc

namespace webrtc {

class Conductor
        : public QObject
        , public PeerConnectionObserver
        , public CreateSessionDescriptionObserver {
    Q_OBJECT

public:
    explicit Conductor(AudioDeviceModule *adm);

    bool open();

    QList<const IceCandidateInterface *> iceCandidates() const;
    const SessionDescriptionInterface *localDescription() const;

    void setRemoteDescription(SessionDescriptionInterface *desc);
    void addIceCandidate(IceCandidateInterface *ice_candidate);

    virtual void close();

Q_SIGNALS:
    void iceCandidateAppeared(const IceCandidateInterface *candidate);
    void localDescriptionAppeared(const SessionDescriptionInterface *desc);

protected:
    ~Conductor();

    bool createPeerConnection();
    void deletePeerConnection();
    void addStreams();

    //
    // PeerConnectionObserver implementation.
    //
    virtual void OnAddStream(MediaStreamInterface *stream);
    virtual void OnRemoveStream(MediaStreamInterface *stream);
    virtual void OnDataChannel(DataChannelInterface *channel);
    virtual void OnRenegotiationNeeded();
    virtual void OnIceConnectionChange(
            PeerConnectionInterface::IceConnectionState new_state);
    virtual void OnIceCandidate(const IceCandidateInterface *candidate);


    //
    // CreateSessionDescriptionObserver implementation.
    //
    virtual void OnSuccess(SessionDescriptionInterface *desc);
    virtual void OnFailure(const std::string &error);

private:
    void sendIceCandidates();

    rtc::Thread *m_signaling_thread;
    bool m_wraps_signaling_thread;
    QScopedPointer<rtc::Thread> m_worker_thread;

    rtc::scoped_refptr<PeerConnectionInterface> m_peer_connection;
    rtc::scoped_refptr<PeerConnectionFactoryInterface>
        m_peer_connection_factory;

    rtc::scoped_refptr<MediaStreamInterface> m_stream;
};

} // namespace webrtc

#endif  // WEBRTC_CONDUCTOR_H
