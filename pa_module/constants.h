#ifndef CONSTANTS_H
#define CONSTANTS_H

#define SAMPLE_RATE_HZ 44100
#define BIT_RATE_BPS   128000
#define NUM_CANNELS    1

#define OUT_PATH                 "/tmp/pacc/"
#define MASTER_PLAYLIST_FILENAME "master.m3u8"
#define PLAYLIST_FILENAME        "pacc.m3u8"

#define CONTROL_SERVER_PORT      51348
#define STREAM_SERVER_PORT       51349

#ifdef QT_DEBUG
#define CORS_ALLOW_ORIGIN "dev.pacc.gorrion.ch"
#else
#define CORS_ALLOW_ORIGIN "pacc.gorrion.ch"
#endif

#endif // CONSTANTS_H

