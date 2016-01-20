# These paths must point to the root of the pulseaudio and webrtc directories.
#
# Pulseaudio: Get the source from http://freedesktop.org/software/pulseaudio/releases/
#             Check how your distro built the library - do a call to ./configure
#
# WebRTC:     Check out http://www.webrtc.org/native-code/development for how to
#             get and build the source.

PULSEAUDIO_VERSION = 7.1
PULSEAUDIO_BASE = $$PWD/../third_party/pulseaudio-$${PULSEAUDIO_VERSION}
WEBRTC_BASE = $$PWD/../third_party/webrtc


INCLUDEPATH += \
    $${PULSEAUDIO_BASE} \
    $${PULSEAUDIO_BASE}/src \
    $${WEBRTC_BASE}/src \
