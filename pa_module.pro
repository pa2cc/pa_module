VERSION = 0.0.1

TARGET = pa_module
TEMPLATE = lib

CONFIG += \
    c++11 \

QT = \
    core \
    network \
    websockets \

DEFINES += \
    LIBRARY_VERSION=\\\"$${VERSION}\\\" \
    QT_FORCE_ASSERTS \
    QT_NO_SIGNALS_SLOTS_KEYWORDS \ # Conflicts with PA where slots is used as a variable name.

PRE_TARGETDEPS += \
    webrtc/libwebrtc.a

LIBS += \
    -L$${OUT_PWD}/webrtc -lwebrtc \

INCLUDEPATH += \
    webrtc/ \

HEADERS += \
    control_server.h \
    pa_module.h\
    pa_sink.h \
    pa_sink_priv.h \
    websocket_server.h \
    writer.h \
    writer_base.h \

SOURCES += \
    pa_module.cpp \
    pa_sink.cpp \
    websocket_server.cpp \
    writer_base.cpp

debug {
    DEFINES += WEBSOCKET_CERT_PATH=\\\"$$PWD/res\\\"
}

include(third_party.pri)
include(webrtc/webrtc.pri)

unix {
    target.path = /usr/lib/pulse-$${PULSEAUDIO_VERSION}/modules/
    INSTALLS += target
}
