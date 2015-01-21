#include "writer_hls.h"

#include "constants.h"

namespace {
const int kHlsTimePerSegmentS = 3;
const int kHlsListSize = 4;
const int kHlsWrap = kHlsListSize*2 + 2;
} // namespace


static AVDictionary *format_options() {
    // Sets the HLS options.
    AVDictionary *options = NULL;
    int ret;
    ret = av_dict_set_int(&options,"hls_time", kHlsTimePerSegmentS, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "hls_list_size", kHlsListSize, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "hls_wrap", kHlsWrap, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");
    ret = av_dict_set_int(&options, "hls_allow_cache", 0, 0);
    Q_ASSERT(ret >= 0 && "Could not set property");

    return options;
}

HLSWriter::HLSWriter()
    : BaseWriter("hls", QString(Stream::kOutPath) + Stream::kPlaylistFilename,
                 AV_CODEC_ID_AUTO, format_options())
{
}
