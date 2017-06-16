#ifndef WEBRTC_WRITER_WEBRTC_H
#define WEBRTC_WRITER_WEBRTC_H

#include <QtCore/QObject>
#include <QtCore/QScopedPointer>

#include "writer_base.h"

class ControlServer;
class WebRTCWriterPriv;

class WebRTCWriter : public BaseWriter {
public:
    WebRTCWriter(PASink *pa_sink, ControlServer *control_server);
    ~WebRTCWriter();

    pa_sample_format_t sampleFormat() const override;
    ssize_t write(const void *buf, size_t length) override;

private:
    QScopedPointer<WebRTCWriterPriv> d;
};

#endif // WEBRTC_WRITER_WEBRTC_H
