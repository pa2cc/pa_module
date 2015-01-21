#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Audio {
const int kSampleRateHz = 44100;
const int kBitRateBps   = 128000;
const int kNumChannels  = 1;
} // namespace Audio

namespace Stream {
const char kOutPath[]                = "/tmp/pacc/";
const char kMasterPlaylistFilename[] = "master.m3u8";
const char kPlaylistFilename[]       = "pacc.m3u8";

const int kControlServerPort = 51348;
const int kStreamServerPort  = 51349;
} // namespace Stream

namespace CORS {
#ifdef QT_DEBUG
const char kAllowOrigin[] = "http://dev.pacc.gorrion.ch";
#else
const char kAllowOrigin[] = "http://pacc.gorrion.ch";
#endif
} // namespace CORS

#endif // CONSTANTS_H

