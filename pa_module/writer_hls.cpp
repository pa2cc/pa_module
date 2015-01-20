#include "writer_hls.h"

#include "constants.h"

#define HLS_TIME_PER_SEGMENT_S 3
#define HLS_LIST_SIZE 4
#define HLS_WRAP HLS_LIST_SIZE*2 + 2

static AVDictionary *format_options() {
    // Sets the HLS options.
    AVDictionary *options = NULL;
    int ret;
    ret = av_dict_set_int(&options,"hls_time", HLS_TIME_PER_SEGMENT_S, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "hls_list_size", HLS_LIST_SIZE, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "hls_wrap", HLS_WRAP, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "hls_allow_cache", 0, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");

    return options;
}

HLSWriter::HLSWriter()
    : BaseWriter("hls", OUT_PATH PLAYLIST_FILENAME, AV_CODEC_ID_AUTO,
                 format_options())
{
}
