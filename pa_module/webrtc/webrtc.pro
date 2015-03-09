TEMPLATE = lib
CONFIG+= staticlib

TARGET = webrtc
DEPENDPATH += .

QT = \
    core \

CONFIG += \
    c++11

DEFINES += \
    QT_NO_EMIT \ # Conflicts with webrtc where emit is used as a function name.
    QT_NO_SIGNALS_SLOTS_KEYWORDS \ # Conflicts with PA where slots is used as a variable name.

DEFINES += \
    V8_DEPRECATION_WARNINGS \
    EXPAT_RELATIVE_PATH \
    FEATURE_ENABLE_VOICEMAIL \
    GTEST_RELATIVE_PATH \
    JSONCPP_RELATIVE_PATH \
    LOGGING=1 \
    SRTP_RELATIVE_PATH \
    FEATURE_ENABLE_SSL \
    FEATURE_ENABLE_PSTN \
    HAVE_SCTP \
    HAVE_SRTP \
    HAVE_WEBRTC_VIDEO \
    HAVE_WEBRTC_VOICE \
    _FILE_OFFSET_BITS=64 \
    CHROMIUM_BUILD \
    CR_CLANG_REVISION=223108 \
    TOOLKIT_VIEWS=1 \
    UI_COMPOSITOR_IMAGE_TRANSPORT \
    USE_AURA=1 \
    USE_ASH=1 \
    USE_PANGO=1 \
    USE_CAIRO=1 \
    USE_DEFAULT_RENDER_THEME=1 \
    USE_LIBJPEG_TURBO=1 \
    USE_X11=1 \
    USE_CLIPBOARD_AURAX11=1 \
    ENABLE_ONE_CLICK_SIGNIN \
    ENABLE_PRE_SYNC_BACKUP \
    ENABLE_REMOTING=1 \
    ENABLE_WEBRTC=1 \
    ENABLE_PEPPER_CDMS \
    ENABLE_CONFIGURATION_POLICY \
    ENABLE_NOTIFICATIONS \
    USE_UDEV \
    DONT_EMBED_BUILD_METADATA \
    ENABLE_TASK_MANAGER=1 \
    ENABLE_EXTENSIONS=1 \
    ENABLE_PLUGINS=1 \
    ENABLE_SESSION_SERVICE=1 \
    ENABLE_THEMES=1 \
    ENABLE_AUTOFILL_DIALOG=1 \
    ENABLE_BACKGROUND=1 \
    ENABLE_GOOGLE_NOW=1 \
    CLD_VERSION=2 \
    ENABLE_PRINTING=1 \
    ENABLE_BASIC_PRINTING=1 \
    ENABLE_PRINT_PREVIEW=1 \
    ENABLE_SPELLCHECK=1 \
    ENABLE_CAPTIVE_PORTAL_DETECTION=1 \
    ENABLE_APP_LIST=1 \
    ENABLE_SETTINGS_APP=1 \
    ENABLE_SUPERVISED_USERS=1 \
    ENABLE_MDNS=1 \
    ENABLE_SERVICE_DISCOVERY=1 \
    V8_USE_EXTERNAL_STARTUP_DATA \
    LIBPEERCONNECTION_LIB=1 \
    LINUX \
    WEBRTC_LINUX \
    HASH_NAMESPACE=__gnu_cxx \
    POSIX \
    WEBRTC_POSIX \
    DISABLE_DYNAMIC_CAST \
    _REENTRANT \
    USE_LIBPCI=1 \
    USE_GLIB=1 \
    USE_NSS=1 \

CONFIG(release, debug|release) {
    DEFINES += \
        NDEBUG \
        NVALGRIND \
        DYNAMIC_ANNOTATIONS_ENABLED=0
}
CONFIG(debug, debug|release) {
    DEFINES += \
        DYNAMIC_ANNOTATIONS_ENABLED=1 \
        WTF_USE_DYNAMIC_ANNOTATIONS=1 \
        _DEBUG \
        _GLIBCXX_DEBUG=1
}

include(webrtc.pri)

INCLUDEPATH +=  \
    . \
    .. \
    ../include/ \
    ../include/pulse/ \
    $$WEBRTC_BASE \
    $$WEBRTC_BASE/third_party \
    $$WEBRTC_BASE/third_party/webrtc \

HEADERS += \
    conductor.h \
    control_server_handler.h \
    pa_audio_device_module.h \
    writer_webrtc.h \
    writer_webrtc_priv.h \

SOURCES += \
    conductor.cc \
    control_server_handler.cpp \
    pa_audio_device_module.cpp \
    writer_webrtc.cpp \

DISTFILES += \
    webrtc.pri
