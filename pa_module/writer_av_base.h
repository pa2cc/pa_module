#ifndef WRITER_AV_BASE_H
#define WRITER_AV_BASE_H

extern "C" {
#include <libavcodec/avcodec.h>
#include "libavformat/avformat.h"
#include <libavutil/dict.h>
}

#include <QtCore/QString>
#include <QtCore/QtGlobal>

#include "writer_base.h"

#define AV_CODEC_ID_AUTO AV_CODEC_ID_NONE

class BaseAVWriter : public BaseWriter {
public:
    BaseAVWriter(PASink *pa_sink, const QString &out_format,
                 const QString &out_filename, AVCodecID audio_codec,
                 AVDictionary *format_options);
    virtual ~BaseAVWriter();

    pa_sample_format_t sampleFormat() const override;
    QString outPath() const;
    QString masterPlaylistFilename() const;
    QString playlistFilename() const;

    ssize_t write(const void *buf, size_t length) override;

private:
    AVStream *add_audio_stream(AVFormatContext *context, AVCodecID codec_id);

    AVFormatContext *m_context;
    AVStream *m_audio_stream;

    AVFrame *m_frame;
    size_t m_num_bytes_in_frame;

    Q_DISABLE_COPY(BaseAVWriter)
};

#endif // WRITER_AV_BASE_H
