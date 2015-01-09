#include "writer_hls.h"

#include <cstdio>

extern "C" {
#include <libavutil/avstring.h>
#include <libavutil/opt.h>
}

#include <QDebug>
#include <QDir>

#include "constants.h"

#define HLS_TIME_PER_SEGMENT_S 3
#define HLS_LIST_SIZE 4
#define HLS_WRAP HLS_LIST_SIZE*2 + 2

static AVStream *add_audio_stream(AVFormatContext *context, AVCodecID codec_id);

void debug_cb(void *avcl, int level, const char *fmt, va_list vl) {
    Q_UNUSED(avcl);

    if (level > AV_LOG_WARNING) {
        return;
    }

    QString line = QString().vsprintf(fmt, vl);
    line.remove(QRegExp("\\n$"));
    qDebug() << QString("PACC: %1").arg(line);
}

HLSWriter::HLSWriter() {
    // Creates the output path if it does not exist yet.
    Q_ASSERT(QDir().mkpath(OUT_PATH) &&
             "Could not create the output directory.");

    // Initializes libavcodec and registers all codecs and formats.
    av_register_all();

    av_log_set_callback(debug_cb);
//    av_log_set_level(AV_LOG_WARNING);

    // Loads the hls output format.
    AVOutputFormat *format = av_guess_format("hls", NULL, NULL);
    Q_ASSERT(format && "Could not find HLS output format");

    // Allocates the output media context.
    m_context = avformat_alloc_context();
    Q_ASSERT(m_context && "Could not allocate the media context");
    m_context->oformat = format;
    av_strlcpy(m_context->filename, OUT_PATH PLAYLIST_FILENAME,
               sizeof(m_context->filename));

    // Adds the audio stream using the default format codec and initializes it.
    Q_ASSERT(format->audio_codec != CODEC_ID_NONE);
    m_audio_stream = add_audio_stream(m_context, format->audio_codec);

    // Opens the encoder.
    int ret = avcodec_open2(m_audio_stream->codec, m_audio_stream->codec->codec,
                            NULL);
    Q_ASSERT(ret >= 0 && "Could not open codec");

    // Frame containing input raw audio.
    m_frame = av_frame_alloc();
    Q_ASSERT(m_frame && "Could not allocate audio frame");

    AVCodecContext *audio_context = m_audio_stream->codec;
    m_frame->nb_samples = audio_context->frame_size;
    m_frame->format = audio_context->sample_fmt;
    m_frame->channel_layout = audio_context->channel_layout;

    // The codec gives us the frame size, in samples, we calculate the size of
    // the samples buffer in bytes.
    m_samples_size = av_samples_get_buffer_size(NULL, audio_context->channels,
                                                audio_context->frame_size,
                                                audio_context->sample_fmt,
                                                0);
    Q_ASSERT(m_samples_size >= 0 &&  "Could not get the sample buffer size");

    // Initializes the sample buffer.
    m_samples = av_malloc(m_samples_size);
    Q_ASSERT(m_samples && "Could not allocate the samples buffer");

    // Sets up the data pointers in the AVFrame.
    ret = avcodec_fill_audio_frame(m_frame, audio_context->channels,
                                   audio_context->sample_fmt,
                                   (const uint8_t*)m_samples, m_samples_size,
                                   0);
    Q_ASSERT(ret >= 0 && "Could not setup audio frame");

    // Sets the HLS options.
    AVDictionary *options = NULL;
    ret = av_dict_set_int(&options,"hls_time", HLS_TIME_PER_SEGMENT_S, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "hls_list_size", HLS_LIST_SIZE, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "hls_wrap", HLS_WRAP, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "hls_allow_cache", 0, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");

    // Initializes the output media and writes the stream header.
    ret = avformat_write_header(m_context, &options);
    Q_ASSERT(ret >= 0 && "Could not write the stream header");
    av_dict_free(&options);
}

HLSWriter::~HLSWriter() {
    // Writes the trailer.
    av_write_trailer(m_context);

    // Closes the codec.
    avcodec_close(m_audio_stream->codec);
    av_freep(m_samples);
    av_frame_free(&m_frame);

    // Frees the format context.
    avformat_free_context(m_context);

    // Removes the temporary files.
    QDir dir(OUT_PATH);
    dir.setNameFilters({PLAYLIST_FILENAME, "*.ts"});
    dir.setFilter(QDir::Files);
    foreach(QString file, dir.entryList()) {
        dir.remove(file);
    }
    dir.rmpath(".");
}

/**
 * Adds an audio output stream.
 */
AVStream *add_audio_stream(AVFormatContext *context, AVCodecID codec_id) {
    AVCodec *codec = avcodec_find_encoder(codec_id);
    Q_ASSERT(codec && "Codec not found");

    AVStream *stream = avformat_new_stream(context, codec);
    Q_ASSERT(stream && "Could not allocate the audio stream");

    // Puts sample parameters */
    AVCodecContext *c = stream->codec;
    c->bit_rate = BIT_RATE_BPS;
    c->sample_rate = SAMPLE_RATE_HZ;
    c->sample_fmt = AV_SAMPLE_FMT_FLTP;
#if NUM_CANNELS == 1
    c->channel_layout = AV_CH_LAYOUT_MONO;
#elif NUM_CHANNELS == 2
    c->channel_layout = AV_CH_LAYOUT_STEREO;
#else
    Q_ASSERT(false);
#endif
    c->channels = av_get_channel_layout_nb_channels(c->channel_layout);
    c->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

    // Some formats want stream headers to be separate.
    if(context->oformat->flags & AVFMT_GLOBALHEADER) {
        c->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    return stream;
}

ssize_t HLSWriter::write(const void *buf, size_t count) {
    // TODO: Remove the copy by only appending if necessary.
    m_buffer.append((const char *)buf, count);

    if (m_buffer.size() < m_samples_size) {
        // There is not enough data to write the next frame.
        return count;
    }

    while (m_buffer.size() >= m_samples_size) {
        // Initializes the sound packet.
        AVPacket pkt;
        av_init_packet(&pkt);
        pkt.data = NULL; // The packet data will be allocated by the encoder.
        pkt.size = 0;

        //debug(QString("Buffer size: %1\n").arg(m_buffer.size()));

        // Copies the data into the samples buffer.
        memcpy(m_samples, m_buffer.data(), m_samples_size);
        m_buffer.remove(0, m_samples_size);

        // Encodes the samples.
        int got_output;
        int ret =  avcodec_encode_audio2(m_audio_stream->codec, &pkt, m_frame,
                                         &got_output);
        Q_ASSERT(ret >= 0 && "Error encoding audio frame");

        // Writes the data into the file if we got some output.
        if (got_output) {
            ret = av_interleaved_write_frame(m_context, &pkt);
            Q_ASSERT(ret >= 0 && "Error while writing audio frame");
        }
    }

    return count;
}
