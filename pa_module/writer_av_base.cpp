#include "writer_av_base.h"

#include <cstdio>

extern "C" {
#include <libavutil/avstring.h>
#include <libavutil/opt.h>
} // extern "C"

#include <QtCore/QDebug>
#include <QtCore/QDir>

#include "pa_sink.h"

namespace {
const QString kOutPath                = "/tmp/pacc/";
const QString kMasterPlaylistFilename = "master.m3u8";
const QString kPlaylistFilename       = "pacc.m3u8";

const pa_sample_format_t kPASampleFormat = PA_SAMPLE_FLOAT32NE;
const AVSampleFormat kAVSampleFormat = AV_SAMPLE_FMT_FLTP;
} // namespace


static void debug_cb(void *avcl, int level, const char *fmt, va_list vl) {
    Q_UNUSED(avcl);

    if (level > AV_LOG_WARNING) {
        return;
    }

    QString line = QString().vsprintf(fmt, vl);
    line.remove(QRegExp("\\n$"));
    qDebug() << QString("PACC: %1").arg(line);
}


BaseAVWriter::BaseAVWriter(PASink *pa_sink, const QString &out_format,
                           const QString &out_filename, AVCodecID audio_codec,
                           AVDictionary *format_options)
    : BaseWriter(pa_sink)
{
    // Creates the output path if it does not exist yet.
    Q_ASSERT(QDir().mkpath(outPath()) &&
             "Could not create the output directory.");

    // Initializes libavcodec and registers all codecs and formats.
    av_register_all();

    av_log_set_callback(debug_cb);
//    av_log_set_level(AV_LOG_WARNING);

    // Loads the output format.
    AVOutputFormat *format =
            av_guess_format(out_format.toStdString().data(), NULL, NULL);
    Q_ASSERT(format && "Could not find the output format");

    // Allocates the output media context.
    m_context = avformat_alloc_context();
    Q_ASSERT(m_context && "Could not allocate the media context");
    m_context->oformat = format;
    av_strlcpy(m_context->filename, out_filename.toStdString().data(),
               sizeof(m_context->filename));

    // Adds the audio stream and initializes it.
    if (audio_codec == AV_CODEC_ID_AUTO) {
        // Loads the default audio codec from the output format.
        audio_codec = format->audio_codec;
        Q_ASSERT(audio_codec != AV_CODEC_ID_NONE &&
                "The output format has no default codec");
    }
    m_audio_stream = add_audio_stream(m_context, audio_codec);

    // Opens the encoder.
    int ret = avcodec_open2(m_audio_stream->codec, m_audio_stream->codec->codec,
                            NULL);
    Q_ASSERT(ret >= 0 && "Could not open codec");

    // Creates the frame containing input raw audio.
    m_frame = av_frame_alloc();
    Q_ASSERT(m_frame && "Could not allocate audio frame");

    AVCodecContext *audio_context = m_audio_stream->codec;
    m_frame->nb_samples = audio_context->frame_size;
    m_frame->format = audio_context->sample_fmt;
    m_frame->channel_layout = audio_context->channel_layout;
    ret = av_frame_get_buffer(m_frame, 0);
    Q_ASSERT(ret >= 0 && "Could not alloc the frame buffer");
    m_num_bytes_in_frame = 0;

    // Initializes the output media and writes the stream header.
    AVDictionary **options = format_options ? &format_options : NULL;
    ret = avformat_write_header(m_context, options);
    Q_ASSERT(ret >= 0 && "Could not write the stream header");
    av_dict_free(options);
}

BaseAVWriter::~BaseAVWriter() {
    // Writes the trailer.
    av_write_trailer(m_context);

    // Closes the codec.
    avcodec_close(m_audio_stream->codec);
    av_frame_free(&m_frame);

    // Frees the format context.
    avformat_free_context(m_context);

    // Removes the temporary files.
    QDir dir(outPath());
    dir.setNameFilters({playlistFilename(), "*.ts"});
    dir.setFilter(QDir::Files);
    foreach(QString file, dir.entryList()) {
        dir.remove(file);
    }
    dir.rmpath(".");
}

pa_sample_format_t BaseAVWriter::sampleFormat() const {
    return kPASampleFormat;
}
QString BaseAVWriter::outPath() const {
    return kOutPath;
}
QString BaseAVWriter::masterPlaylistFilename() const {
    return kMasterPlaylistFilename;
}
QString BaseAVWriter::playlistFilename() const {
    return kPlaylistFilename;
}

/**
 * Adds an audio output stream.
 */
AVStream *BaseAVWriter::add_audio_stream(AVFormatContext *context,
                                         AVCodecID codec_id) {
    AVCodec *codec = avcodec_find_encoder(codec_id);
    Q_ASSERT(codec && "Codec not found");

    AVStream *stream = avformat_new_stream(context, codec);
    Q_ASSERT(stream && "Could not allocate the audio stream");

    // Puts sample parameters */
    AVCodecContext *c = stream->codec;
    c->bit_rate = pa_sink()->bitRateBps();
    c->sample_rate = pa_sink()->sampleRateHz();
    c->sample_fmt = kAVSampleFormat;
    switch (pa_sink()->numChannels()) {
        case 1: c->channel_layout = AV_CH_LAYOUT_MONO; break;
        case 2: c->channel_layout = AV_CH_LAYOUT_STEREO; break;

        default: Q_ASSERT(false);
    }
    c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
    c->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    // Some formats want stream headers to be separate.
    if(context->oformat->flags & AVFMT_GLOBALHEADER) {
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    return stream;
}

static void writeFrame(AVFormatContext *context, AVCodecContext *codec,
                       AVFrame *frame) {
    // Initializes the sound packet.
    AVPacket pkt;
    av_init_packet(&pkt);
    pkt.data = NULL;
    pkt.size = 0;

    // Encodes the samples.
    int got_output;
    int ret = avcodec_encode_audio2(codec, &pkt, frame, &got_output);
    Q_ASSERT(ret >= 0 && "Error encoding audio frame");

    // Writes the frame (if we got some output).
    if (got_output) {
        ret = av_interleaved_write_frame(context, &pkt);
        Q_ASSERT(ret >= 0 && "Error while writing audio frame");
    }
}

ssize_t BaseAVWriter::write(const void *buf, size_t length) {
    const int sample_size_b =
            av_get_bytes_per_sample(m_audio_stream->codec->sample_fmt);
    const size_t frame_size_b = m_frame->linesize[0];

    // The data must be aligned and the frame buffer not be overrun.
    Q_ASSERT(length % (sample_size_b * m_frame->channels) == 0);
    Q_ASSERT(m_num_bytes_in_frame < frame_size_b);

    size_t ate = 0;
    while (ate < length) {
        // Adds the data to the channels.
        while (ate < length && m_num_bytes_in_frame < frame_size_b) {
            for (int c = 0; c < m_frame->channels; ++c) {
                memcpy(&m_frame->data[c][m_num_bytes_in_frame],
                       &((uint8_t *)buf)[ate],
                       sample_size_b);
                ate += sample_size_b;
            }
            m_num_bytes_in_frame += sample_size_b; // Per channel
        }

        // Writes the next frame (if full).
        if (m_num_bytes_in_frame == frame_size_b) {
            writeFrame(m_context, m_audio_stream->codec, m_frame);
            m_num_bytes_in_frame = 0;
        }
    }

    return length;
}
