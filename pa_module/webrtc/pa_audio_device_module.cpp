#include "pa_audio_device_module.h"

#include <cstring>
#include <limits>

#include <QtCore/QDebug>
#include <QtCore/QMutexLocker>
#include <QtCore/QtGlobal>

#include "pa_sink.h"

namespace {
const char kRecordingDeviceName[] = "PACC";
} // namespace

namespace webrtc {

PAAudioDeviceModule::PAAudioDeviceModule(PASink *pa_sink)
    : m_pa_sink(pa_sink)
    , m_recording_initialized(false)
    , m_do_record(false)
    , m_agc(false)
{
    m_audio_device_buffer.SetRecordingSampleRate(0);
    m_audio_device_buffer.SetPlayoutSampleRate(0);
    m_audio_device_buffer.SetRecordingChannels(0);
    m_audio_device_buffer.SetPlayoutChannels(0);
}

ssize_t PAAudioDeviceModule::write(const void *buf, size_t length) {
    QMutexLocker l(&m_mutex);

    if (m_do_record) {
        // Defines the block size that we must consider when sending data to the
        // audio device buffer.
        // From: webrtc/common_audio/resampler/push_resampler.cc
        //       (PushResampler::Resample).
        const size_t block_size =
                m_audio_device_buffer.RecordingSampleRate() * 2 *
                m_audio_device_buffer.RecordingChannels() / 100;

        // Calculates the number of bytes per sample and the sample count per
        // block.
        const size_t bytes_per_sample =
                2 * m_audio_device_buffer.RecordingChannels();
        const uint32_t num_samples_per_block = block_size / bytes_per_sample;

        // Adds the data to the remaining buffer.
        m_remaining_data.append((const char *)buf, length);

        // We send the data in chunks that are digestible by the audio device
        // buffer.
        while ((size_t)m_remaining_data.size() >= block_size) {
            int32_t status = m_audio_device_buffer.SetRecordedBuffer(
                        m_remaining_data.data(), num_samples_per_block);
            if (status == -1) {
                qWarning() << "Failed to set the recorded buffer.";
            }

            status = m_audio_device_buffer.DeliverRecordedData();
            if (status == -1) {
                qWarning() << "Failed to deliver the recorded buffer.";
            }

            // Removes the processed data from the remaining buffer.
            m_remaining_data.remove(0, block_size);
        }
    }
    return length;
}


int64_t PAAudioDeviceModule::TimeUntilNextProcess() {
    qDebug() << __FUNCTION__;
    return std::numeric_limits<int64_t>::max();
}

int32_t PAAudioDeviceModule::Process() {
    qDebug() << __FUNCTION__;
    return 0;
}

// Retrieve the currently utilized audio layer
int32_t PAAudioDeviceModule::ActiveAudioLayer(AudioLayer *audioLayer) const {
    qDebug() << __FUNCTION__;
    *audioLayer = kPlatformDefaultAudio;
    return 0;
}

// Error handling
AudioDeviceModule::ErrorCode PAAudioDeviceModule::LastError() const {
    return kAdmErrNone;
}
int32_t PAAudioDeviceModule::RegisterEventObserver(
        AudioDeviceObserver *eventCallback) {
    qDebug() << __FUNCTION__;
    // We currently do not do any error handling.
    Q_UNUSED(eventCallback);
    return 0;
}

// Full-duplex transportation of PCM audio
int32_t PAAudioDeviceModule::RegisterAudioCallback(
        AudioTransport *audioCallback) {
    qDebug() << __FUNCTION__;
    return m_audio_device_buffer.RegisterAudioCallback(audioCallback);
}

// Main initialization and termination
int32_t PAAudioDeviceModule::Init() {
    qDebug() << __FUNCTION__;
    QMutexLocker l(&m_mutex);

    m_initialized = true;
    return 0;
}
int32_t PAAudioDeviceModule::Terminate() {
    qDebug() << __FUNCTION__;
    QMutexLocker l(&m_mutex);

    m_initialized = false;
    return 0;
}
bool PAAudioDeviceModule::Initialized() const {
    qDebug() << __FUNCTION__;
    QMutexLocker l(&m_mutex);
    return m_initialized;
}

// Device enumeration
int16_t PAAudioDeviceModule::PlayoutDevices() {
    qDebug() << __FUNCTION__;
    return 0; // There are no playout devices.
}
int16_t PAAudioDeviceModule::RecordingDevices() {
    qDebug() << __FUNCTION__;
    return 1;
}
int32_t PAAudioDeviceModule::PlayoutDeviceName(uint16_t index,
                                               char name[kAdmMaxDeviceNameSize],
                                               char guid[kAdmMaxGuidSize]) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(index);

    if (name != NULL) {
        memset(name, 0, kAdmMaxDeviceNameSize);
    }

    if (guid != NULL) {
        memset(guid, 0, kAdmMaxGuidSize);
    }

    return 0;
}
int32_t PAAudioDeviceModule::RecordingDeviceName(
        uint16_t index, char name[kAdmMaxDeviceNameSize],
        char guid[kAdmMaxGuidSize]) {
    qDebug() << __FUNCTION__;
    if (index != 0 || name == NULL) {
        return -1;
    }

    if (guid != NULL) {
        memset(guid, 0, kAdmMaxGuidSize);
    }

    // Sets the name.
    strncpy(name, kRecordingDeviceName, kAdmMaxDeviceNameSize);
    name[kAdmMaxDeviceNameSize - 1] = '\0';

    return 0;
}

// Device selection
int32_t PAAudioDeviceModule::SetPlayoutDevice(uint16_t index) {
    qDebug() << __FUNCTION__ << " " << index;
    Q_UNUSED(index);
    // Currently it is set despite the fact that we promote no playout devices.
    return 0;
}
int32_t PAAudioDeviceModule::SetPlayoutDevice(WindowsDeviceType device) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(device);
    return -1;
}
int32_t PAAudioDeviceModule::SetRecordingDevice(uint16_t index) {
    qDebug() << __FUNCTION__ << " " << index;
    return index == 0 ? 0 : -1;
}
int32_t PAAudioDeviceModule::SetRecordingDevice(WindowsDeviceType device) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(device);
    return -1;
}

// Audio transport initialization
int32_t PAAudioDeviceModule::PlayoutIsAvailable(bool *available) {
    qDebug() << __FUNCTION__;
    *available = false;
    return 0;
}
int32_t PAAudioDeviceModule::InitPlayout() {
    qDebug() << __FUNCTION__;
    return -1;
}
bool PAAudioDeviceModule::PlayoutIsInitialized() const {
    qDebug() << __FUNCTION__;
    return false;
}
int32_t PAAudioDeviceModule::RecordingIsAvailable(bool *available) {
    qDebug() << __FUNCTION__;
    *available = true;
    return 0;
}
int32_t PAAudioDeviceModule::InitRecording() {
    qDebug() << __FUNCTION__;
    QMutexLocker l(&m_mutex);
    m_recording_initialized = true;

    // TODO: get sample rate from PASink
    m_audio_device_buffer.SetRecordingSampleRate(m_pa_sink->sampleRateHz());
    return m_audio_device_buffer.InitRecording();
}
bool PAAudioDeviceModule::RecordingIsInitialized() const {
    qDebug() << __FUNCTION__;
    QMutexLocker l(&m_mutex);
    return m_recording_initialized;
}

// Audio transport control
int32_t PAAudioDeviceModule::StartPlayout() {
    qDebug() << __FUNCTION__;
    return -1;
}
int32_t PAAudioDeviceModule::StopPlayout() {
    qDebug() << __FUNCTION__;
    // Gets called even if no playout is running and we do not even support
    // playout.
    return 0;
}
bool PAAudioDeviceModule::Playing() const {
    qDebug() << __FUNCTION__;
    return false;
}
int32_t PAAudioDeviceModule::StartRecording() {
    qDebug() << __FUNCTION__;
    QMutexLocker l(&m_mutex);
    if (!m_recording_initialized) {
        return -1;
    }

    m_do_record = true;
    return 0;
}
int32_t PAAudioDeviceModule::StopRecording() {
    qDebug() << __FUNCTION__;
    QMutexLocker l(&m_mutex);
    m_do_record = false;
    return 0;
}
bool PAAudioDeviceModule::Recording() const {
    qDebug() << __FUNCTION__;
    QMutexLocker l(&m_mutex);
    return m_do_record;
}

// Microphone Automatic Gain Control (AGC)
int32_t PAAudioDeviceModule::SetAGC(bool enable) {
    qDebug() << __FUNCTION__ << " " << enable;
    m_agc = enable;
    return 0;
}
bool PAAudioDeviceModule::AGC() const {
    qDebug() << __FUNCTION__;
    return m_agc;
}

// Volume control based on the Windows Wave API (Windows only)
int32_t PAAudioDeviceModule::SetWaveOutVolume(uint16_t volumeLeft,
                                              uint16_t volumeRight) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(volumeLeft);
    Q_UNUSED(volumeRight);
    return -1;
}
int32_t PAAudioDeviceModule::WaveOutVolume(uint16_t *volumeLeft,
                                           uint16_t *volumeRight) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(volumeLeft);
    Q_UNUSED(volumeRight);
    return -1;
}

// Audio mixer initialization
int32_t PAAudioDeviceModule::InitSpeaker() {
    qDebug() << __FUNCTION__;
    return 0;
}
bool PAAudioDeviceModule::SpeakerIsInitialized() const {
    qDebug() << __FUNCTION__;
    return true;
}
int32_t PAAudioDeviceModule::InitMicrophone() {
    qDebug() << __FUNCTION__;
    return 0;
}
bool PAAudioDeviceModule::MicrophoneIsInitialized() const {
    qDebug() << __FUNCTION__;
    return true;
}

// Speaker volume controls
int32_t PAAudioDeviceModule::SpeakerVolumeIsAvailable(bool *available) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(available);
    return -1;
}
int32_t PAAudioDeviceModule::SetSpeakerVolume(uint32_t volume) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(volume);
    return -1;
}
int32_t PAAudioDeviceModule::SpeakerVolume(uint32_t *volume) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(volume);
    return -1;
}
int32_t PAAudioDeviceModule::MaxSpeakerVolume(uint32_t *maxVolume) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(maxVolume);
    return -1;
}
int32_t PAAudioDeviceModule::MinSpeakerVolume(uint32_t *minVolume) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(minVolume);
    return -1;
}
int32_t PAAudioDeviceModule::SpeakerVolumeStepSize(uint16_t *stepSize) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(stepSize);
    return -1;
}

// Microphone volume controls
int32_t PAAudioDeviceModule::MicrophoneVolumeIsAvailable(bool *available) {
    qDebug() << __FUNCTION__;
    *available = false;
    return 0;
}
int32_t PAAudioDeviceModule::SetMicrophoneVolume(uint32_t volume) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(volume);
    return -1;
}
int32_t PAAudioDeviceModule::MicrophoneVolume(uint32_t *volume) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(volume);
    return -1;
}
int32_t PAAudioDeviceModule::MaxMicrophoneVolume(uint32_t *maxVolume) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(maxVolume);
    return -1;
}
int32_t PAAudioDeviceModule::MinMicrophoneVolume(uint32_t *minVolume) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(minVolume);
    return -1;
}
int32_t PAAudioDeviceModule::MicrophoneVolumeStepSize(uint16_t *stepSize) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(stepSize);
    return -1;
}

// Speaker mute control
int32_t PAAudioDeviceModule::SpeakerMuteIsAvailable(bool *available) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(available);
    return -1;
}
int32_t PAAudioDeviceModule::SetSpeakerMute(bool enable) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(enable);
    return -1;
}
int32_t PAAudioDeviceModule::SpeakerMute(bool *enabled) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(enabled);
    return -1;
}

// Microphone mute control
int32_t PAAudioDeviceModule::MicrophoneMuteIsAvailable(bool *available) {
    qDebug() << __FUNCTION__;
    *available = false;
    return 0;
}
int32_t PAAudioDeviceModule::SetMicrophoneMute(bool enable) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(enable);
    return -1;
}
int32_t PAAudioDeviceModule::MicrophoneMute(bool *enabled) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(enabled);
    return -1;
}

// Microphone boost control
int32_t PAAudioDeviceModule::MicrophoneBoostIsAvailable(bool *available) {
    qDebug() << __FUNCTION__;
    *available = false;
    return 0;
}
int32_t PAAudioDeviceModule::SetMicrophoneBoost(bool enable) {
    qDebug() << __FUNCTION__;
    Q_ASSERT(enable == false);
    return 0;
}
int32_t PAAudioDeviceModule::MicrophoneBoost(bool *enabled) const {
    qDebug() << __FUNCTION__;
    *enabled = false;
    return 0;
}

// Stereo support
int32_t PAAudioDeviceModule::StereoPlayoutIsAvailable(bool *available) const {
    qDebug() << __FUNCTION__;
    *available = false;
    return 0;
}
int32_t PAAudioDeviceModule::SetStereoPlayout(bool enable) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(enable);
    return 0;
}
int32_t PAAudioDeviceModule::StereoPlayout(bool *enabled) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(enabled);
    return -1;
}

int32_t PAAudioDeviceModule::StereoRecordingIsAvailable(bool *available) const {
    qDebug() << __FUNCTION__;
    *available = m_pa_sink->numChannels() == 2;
    return 0;
}
int32_t PAAudioDeviceModule::SetStereoRecording(bool enable) {
    qDebug() << __FUNCTION__ << " " << enable;
    QMutexLocker l(&m_mutex);
    uint8_t num_channels = enable ? 2 : 1;
    Q_ASSERT(num_channels == m_pa_sink->numChannels());
    m_audio_device_buffer.SetRecordingChannels(num_channels);
    return 0;
}
int32_t PAAudioDeviceModule::StereoRecording(bool *enabled) const {
    qDebug() << __FUNCTION__;
    *enabled = m_audio_device_buffer.RecordingChannels() == 2;
    return 0;
}
int32_t PAAudioDeviceModule::SetRecordingChannel(const ChannelType channel) {
    qDebug() << __FUNCTION__ << " " << channel;
    return m_audio_device_buffer.SetRecordingChannel(channel);
}
int32_t PAAudioDeviceModule::RecordingChannel(ChannelType *channel) const {
    qDebug() << __FUNCTION__;
    return m_audio_device_buffer.RecordingChannel(*channel);
}

// Delay information and control
int32_t PAAudioDeviceModule::SetPlayoutBuffer(const BufferType type,
                                              uint16_t sizeMS) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(type);
    Q_UNUSED(sizeMS);
    return -1;
}
int32_t PAAudioDeviceModule::PlayoutBuffer(BufferType *type,
                                           uint16_t *sizeMS) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(type);
    Q_UNUSED(sizeMS);
    return -1;
}
int32_t PAAudioDeviceModule::PlayoutDelay(uint16_t *delayMS) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(delayMS);
    return -1;
}
int32_t PAAudioDeviceModule::RecordingDelay(uint16_t *delayMS) const {
    qDebug() << __FUNCTION__;
    // TODO
    *delayMS = 0;
    return 0;
}

// CPU load
int32_t PAAudioDeviceModule::CPULoad(uint16_t *load) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(load);
    return -1;
}

// Recording of raw PCM data
int32_t PAAudioDeviceModule::StartRawOutputFileRecording(
        const char pcmFileNameUTF8[kAdmMaxFileNameSize]) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(pcmFileNameUTF8);
    return -1;
}
int32_t PAAudioDeviceModule::StopRawOutputFileRecording() {
    qDebug() << __FUNCTION__;
    return -1;
}
int32_t PAAudioDeviceModule::StartRawInputFileRecording(
        const char pcmFileNameUTF8[kAdmMaxFileNameSize]) {
    qDebug() << __FUNCTION__;
    return m_audio_device_buffer.StartInputFileRecording(pcmFileNameUTF8);
}
int32_t PAAudioDeviceModule::StopRawInputFileRecording() {
    qDebug() << __FUNCTION__;
    return m_audio_device_buffer.StopInputFileRecording();
}

// Native sample rate controls (samples/sec)
int32_t PAAudioDeviceModule::SetRecordingSampleRate(
        const uint32_t samplesPerSec) {
    qDebug() << __FUNCTION__;
    // TODO: could be done, right?
    Q_UNUSED(samplesPerSec);
    return -1;
}
int32_t PAAudioDeviceModule::RecordingSampleRate(
        uint32_t *samplesPerSec) const {
    qDebug() << __FUNCTION__;
    *samplesPerSec = m_audio_device_buffer.RecordingSampleRate();
    return 0;
}

int32_t PAAudioDeviceModule::SetPlayoutSampleRate(
        const uint32_t samplesPerSec) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(samplesPerSec);
    return -1;
}
int32_t PAAudioDeviceModule::PlayoutSampleRate(uint32_t *samplesPerSec) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(samplesPerSec);
    return -1;
}

// Mobile device specific functions
int32_t PAAudioDeviceModule::ResetAudioDevice() {
    qDebug() << __FUNCTION__;
    return -1;
}
int32_t PAAudioDeviceModule::SetLoudspeakerStatus(bool enable) {
    qDebug() << __FUNCTION__;
    Q_UNUSED(enable);
    return -1;
}
int32_t PAAudioDeviceModule::GetLoudspeakerStatus(bool *enabled) const {
    qDebug() << __FUNCTION__;
    Q_UNUSED(enabled);
    return -1;
    return -1;
}

} // namespace webrtc
