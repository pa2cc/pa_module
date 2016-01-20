#include "conductor.h"

#include <utility>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include <talk/app/webrtc/test/fakeconstraints.h>
#include <webrtc/base/common.h>
#include <webrtc/base/logging.h>
#include <webrtc/base/thread.h>
#pragma GCC diagnostic pop

#include <QtCore/QDebug>
#include <QtCore/QtGlobal>

namespace {
const QString kStunServer = "stun:stun.l.google.com:19302";
const QString kAudioLabel = "audio";
const QString kStreamLabel = "stream";
} // namespace

namespace webrtc {

class DummySetSessionDescriptionObserver : public SetSessionDescriptionObserver {
public:
    static DummySetSessionDescriptionObserver *create() {
        return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
    }

    virtual void OnSuccess() {
    }

    virtual void OnFailure(const std::string &error) {
        qWarning() << "Could not set the session description: "
                   << QString::fromStdString(error);
    }

protected:
    DummySetSessionDescriptionObserver() {}
    ~DummySetSessionDescriptionObserver() {}
};


Conductor::Conductor(AudioDeviceModule *adm)
    : m_signaling_thread(rtc::ThreadManager::Instance()->CurrentThread())
    , m_wraps_signaling_thread(false)
    , m_worker_thread(new rtc::Thread)
{
    freopen("/tmp/stdout.log", "w", stdout);
    freopen("/tmp/stderr.log", "w", stderr);
    m_worker_thread->Start();

    if (!m_signaling_thread) {
        m_signaling_thread =
                rtc::ThreadManager::Instance()->WrapCurrentThread();
        m_wraps_signaling_thread = true;
    }
    m_peer_connection_factory = CreatePeerConnectionFactory(
                m_worker_thread.data(), m_signaling_thread, adm, NULL, NULL);
    Q_ASSERT(m_peer_connection_factory.get() &&
             "Failed to initialize PeerConnectionFactory");
}

Conductor::~Conductor() {
    close();

    if (m_wraps_signaling_thread) {
        rtc::ThreadManager::Instance()->UnwrapCurrentThread();
    }
}

void Conductor::close() {
    deletePeerConnection();
}

bool Conductor::open() {
    if (m_peer_connection.get()) {
        return true;
    }

    Q_ASSERT(createPeerConnection() &&
             "CreatePeerConnection failed");

    addStreams();

    FakeConstraints constraints;
    constraints.AddMandatory(
                MediaConstraintsInterface::kOfferToReceiveAudio, false);
    constraints.AddOptional(
                MediaConstraintsInterface::kOfferToReceiveVideo, false);
    m_peer_connection->CreateOffer(this, &constraints);

    return m_peer_connection.get() != NULL;
}

bool Conductor::createPeerConnection() {
    Q_ASSERT(!m_peer_connection.get());

    PeerConnectionInterface::RTCConfiguration rtc_config;
    PeerConnectionInterface::IceServer server;
    server.uri = kStunServer.toStdString();
    rtc_config.servers.push_back(server);

    FakeConstraints constraints;
    constraints.AddOptional(MediaConstraintsInterface::kEnableDtlsSrtp, true);

    m_peer_connection = m_peer_connection_factory->CreatePeerConnection(
                rtc_config, &constraints, NULL, NULL, this);
    return m_peer_connection.get() != NULL;
}

void Conductor::addStreams() {
    Q_ASSERT(!m_stream.get());

    // Specifies the desired audio options.
    FakeConstraints audio_constraints;
    audio_constraints.AddMandatory(MediaConstraintsInterface::kEchoCancellation,
                                   MediaConstraintsInterface::kValueFalse);
    audio_constraints.AddMandatory(
                MediaConstraintsInterface::kExperimentalEchoCancellation,
                MediaConstraintsInterface::kValueFalse);
    audio_constraints.AddMandatory(MediaConstraintsInterface::kAutoGainControl,
                                   MediaConstraintsInterface::kValueFalse);
    audio_constraints.AddMandatory(MediaConstraintsInterface::kNoiseSuppression,
                                   MediaConstraintsInterface::kValueFalse);
    audio_constraints.AddMandatory(
                MediaConstraintsInterface::kExperimentalNoiseSuppression,
                MediaConstraintsInterface::kValueFalse);
    audio_constraints.AddMandatory(MediaConstraintsInterface::kHighpassFilter,
                                   MediaConstraintsInterface::kValueFalse);
    audio_constraints.AddMandatory(
                MediaConstraintsInterface::kTypingNoiseDetection,
                MediaConstraintsInterface::kValueFalse);
    audio_constraints.AddMandatory(MediaConstraintsInterface::kAudioMirroring,
                                   MediaConstraintsInterface::kValueFalse);

    // Creates the audio track.
    rtc::scoped_refptr<AudioSourceInterface> audio_source(
                m_peer_connection_factory->CreateAudioSource(
                    &audio_constraints));
    rtc::scoped_refptr<AudioTrackInterface> audio_track(
                m_peer_connection_factory->CreateAudioTrack(
                    kAudioLabel.toStdString(), audio_source));

    // Creates the media stream.
    m_stream = m_peer_connection_factory->CreateLocalMediaStream(
                kStreamLabel.toStdString());
    m_stream->AddTrack(audio_track);

    // Adds the stream to the peer connection.
    if (!m_peer_connection->AddStream(m_stream)) {
        qWarning() << "Adding stream to PeerConnection failed";
    }
}

void Conductor::deletePeerConnection() {
    m_peer_connection = NULL;
    m_stream = NULL;
}

const SessionDescriptionInterface *Conductor::localDescription() const {
    return m_peer_connection->local_description();
}
QList<const IceCandidateInterface *> Conductor::iceCandidates() const {
    QList<const IceCandidateInterface *> ret;

    // Reads the local ice candidates.
    const SessionDescriptionInterface *local_description = localDescription();
    if (local_description) {
        size_t m_num = local_description->number_of_mediasections();
        for (size_t mid = 0; mid < m_num; ++mid) {
            const IceCandidateCollection *candidates =
                    local_description->candidates(mid);
            for (size_t cid = 0; cid < candidates->count(); ++cid) {
                ret.append(candidates->at(cid));
            }
        }
    }

    return ret;
}

void Conductor::setRemoteDescription(SessionDescriptionInterface *desc) {
    m_peer_connection->SetRemoteDescription(
                DummySetSessionDescriptionObserver::create(), desc);
}

void Conductor::addRemoteIceCandidate(IceCandidateInterface *ice_candidate) {
    m_peer_connection->AddIceCandidate(ice_candidate);
}

//
// PeerConnectionObserver implementation.
//

// Called when a remote stream is added
void Conductor::OnAddStream(MediaStreamInterface *stream) {
    Q_UNUSED(stream);
}
void Conductor::OnRemoveStream(MediaStreamInterface *stream) {
    Q_UNUSED(stream);
}
void Conductor::OnDataChannel(DataChannelInterface *channel) {
    Q_UNUSED(channel);
}

void Conductor::OnRenegotiationNeeded() {
}

void Conductor::OnIceConnectionChange(
        PeerConnectionInterface::IceConnectionState new_state) {
    Q_UNUSED(new_state);
}
void Conductor::OnIceCandidate(const IceCandidateInterface *candidate) {
    Q_EMIT iceCandidateAppeared(candidate);
}

//
// CreateSessionDescriptionObserver implementation.
//

void Conductor::OnSuccess(SessionDescriptionInterface *desc) {
    m_peer_connection->SetLocalDescription(
                DummySetSessionDescriptionObserver::create(), desc);

    // TODO: move to DummySetSessionDescriptionObserver::OnSuccess
    Q_EMIT localDescriptionAppeared(desc);
}

void Conductor::OnFailure(const std::string &error) {
    qWarning() << "Could not create offer: " << QString::fromStdString(error);
}

} // namespace webrtc
