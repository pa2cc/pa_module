#ifndef CONSTANTS_H
#define CONSTANTS_H

#define SAMPLE_RATE_HZ 44100
#define BIT_RATE_BPS   128000
#define NUM_CANNELS    1

#define OUT_PATH                 "/tmp/pacc/"
#define MASTER_PLAYLIST_FILENAME "master.m3u8"
#define PLAYLIST_FILENAME        "pacc.m3u8"

// Must not end with a slash.
#define HTTP_CONTROL_PREFIX      "/control"
#define HTTP_CONTROL_STREAM_URL  HTTP_CONTROL_PREFIX "/streamUrl"

#define HTTP_STREAM_PREFIX       "/stream"
#define HTTP_MASTER_PLAYLIST_URL HTTP_STREAM_PREFIX "/" MASTER_PLAYLIST_FILENAME

#endif // CONSTANTS_H

