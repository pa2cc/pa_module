#include "writer_adts.h"

#include <QDebug>
#include <QDir>

#include "constants.h"

#define HLS_TIME_PER_SEGMENT_S 1
#define HLS_LIST_SIZE 3
#define HLS_WRAP HLS_LIST_SIZE*2 + 2
#define HLS_TS_FILENAME_TEMPLATE "pacc_%03d.ts"

static AVDictionary *format_options() {
    // Sets the segment format options.
    AVDictionary *options = NULL;
    int ret;
    ret = av_dict_set(&options,"segment_format", "adts", 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set(&options, "segment_list_type", "hls", 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set(&options, "segment_list", OUT_PATH PLAYLIST_FILENAME, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "segment_time", HLS_TIME_PER_SEGMENT_S, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set(&options, "segment_list_flags", "live", 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "segment_list_size", HLS_LIST_SIZE, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "segment_wrap", HLS_WRAP, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");

    return options;
}

ADTSWriter::ADTSWriter()
    : BaseWriter("segment", OUT_PATH HLS_TS_FILENAME_TEMPLATE, AV_CODEC_ID_AAC,
                 format_options())
{
}
