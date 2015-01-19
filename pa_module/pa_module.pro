VERSION = 0.0.1

TARGET = pa_module
TEMPLATE = lib

CONFIG += \
    c++11 \
    TUFAO1 \

QT = \
    core \
    network \

DEFINES += \
    QT_FORCE_ASSERTS \
    LIBRARY_VERSION=\\\"$${VERSION}\\\" \

LIBS += \
    -lavcodec \
    -lavformat \

INCLUDEPATH += \
    include/ \

HEADERS += \
    change_notifier.h \
    constants.h \
    control_server.h \
    pa_module.h\
    pa_sink.h \
    streaming_server.h \
    writer.h \
    writer_hls.h \

SOURCES += \
    control_server.cpp \
    pa_module.cpp \
    pa_sink.cpp \
    streaming_server.cpp \
    writer_hls.cpp \

unix {
    target.path = /usr/lib/pulse-5.0/modules/
    INSTALLS += target
}
