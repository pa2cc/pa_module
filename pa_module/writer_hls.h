#ifndef WRITER_HLS_H
#define WRITER_HLS_H

extern "C" {
#include <libavcodec/avcodec.h>
#include "libavformat/avformat.h"
}

#include <QByteArray>
#include <QtGlobal>

#include "writer.h"

class HLSWriter : public Writer {
public:
    HLSWriter();
    virtual ~HLSWriter();

    ssize_t write(const void *buf, size_t count) override;

private:
    AVFormatContext *m_context;
    AVStream *m_audio_stream;

    AVFrame *m_frame;
    ssize_t m_samples_size;
    void *m_samples;

    QByteArray m_buffer;

    Q_DISABLE_COPY(HLSWriter)
};

#endif // WRITER_HLS_H

