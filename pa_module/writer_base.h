#ifndef WRITER_BASE_H
#define WRITER_BASE_H

extern "C" {
#include <libavcodec/avcodec.h>
#include "libavformat/avformat.h"
#include <libavutil/dict.h>
}

#include <QByteArray>
#include <QString>
#include <QtGlobal>

#include "writer.h"

#define AV_CODEC_ID_AUTO AV_CODEC_ID_NONE

class BaseWriter : public Writer {
public:
    BaseWriter(const QString &out_format, const QString &out_filename,
               AVCodecID audio_codec, AVDictionary *format_options);
    virtual ~BaseWriter();

    ssize_t write(const void *buf, size_t count) override;

private:
    AVFormatContext *m_context;
    AVStream *m_audio_stream;

    AVFrame *m_frame;
    ssize_t m_samples_size;
    void *m_samples;

    QByteArray m_buffer;

    Q_DISABLE_COPY(BaseWriter)
};

#endif // WRITER_BASE_H
