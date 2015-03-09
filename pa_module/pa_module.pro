VERSION = 0.0.1

TARGET = pa_module
TEMPLATE = lib

CONFIG += \
    c++11 \
    TUFAO1 \

QT = \
    core \
    network \
    websockets \

DEFINES += \
    QT_FORCE_ASSERTS \
    QT_NO_SIGNALS_SLOTS_KEYWORDS \ # Conflicts with PA where slots is used as a variable name.
    LIBRARY_VERSION=\\\"$${VERSION}\\\" \

PRE_TARGETDEPS += \
    webrtc/libwebrtc.a

LIBS += \
    -lavcodec \
    -lavformat \
    -L$${OUT_PWD}/webrtc -lwebrtc \

INCLUDEPATH += \
    include/ \
    include/pulse/ \
    webrtc/ \

HEADERS += \
    change_notifier.h \
    control_server.h \
    http_control_server.h \
    http_streaming_server.h \
    pa_module.h\
    pa_sink.h \
    websocket_server.h \
    writer.h \
    writer_adts.h \
    writer_hls.h \
    writer_av_base.h \
    writer_base.h

SOURCES += \
    change_notifier.cpp \
    http_control_server.cpp \
    http_streaming_server.cpp \
    pa_module.cpp \
    pa_sink.cpp \
    websocket_server.cpp \
    writer_adts.cpp \
    writer_hls.cpp \
    writer_av_base.cpp \
    writer_base.cpp

RESOURCES += \
    res/pa_sink.qrc

unix {
    target.path = /usr/lib/pulse-6.0/modules/
    INSTALLS += target
}

include(webrtc/webrtc.pri)
