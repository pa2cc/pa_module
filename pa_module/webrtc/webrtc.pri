# Either set this to the correct path or run qmake like this:
#   qmake WEBRTC_BASE=/path/to/webrtc/trunk
#WEBRTC_BASE = /path/to/webrtc/trunk/
WEBRTC_BASE = $$(WEBRTC_BASE) # Reads the WEBRTC_BASE environment variable.
isEmpty(WEBRTC_BASE){
  error(Please point the WEBRTC_BASE variable to your webrtc root.)
}

CONFIG(debug,debug|release) {
    WEBRTC_BUILD = $$WEBRTC_BASE/out/Debug
} else {
    WEBRTC_BUILD = $$WEBRTC_BASE/out/Release
}

LIBS += \
    -Wl,--start-group \
    $$WEBRTC_BUILD/obj/talk/libjingle_peerconnection.a \
    $$WEBRTC_BUILD/obj/webrtc/base/librtc_base.a \
    $$WEBRTC_BUILD/obj/webrtc/libwebrtc_common.a \
    $$WEBRTC_BUILD/obj/webrtc/base/librtc_base_approved.a \
    $$WEBRTC_BUILD/obj/chromium/src/net/third_party/nss/libcrssl.a \
    $$WEBRTC_BUILD/obj/chromium/src/net/third_party/nss/libcrssl.a \
    $$WEBRTC_BUILD/obj/talk/libjingle_media.a \
    $$WEBRTC_BUILD/libyuv.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/libjpeg_turbo/libjpeg_turbo.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/usrsctp/libusrsctplib.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libvideo_render_module.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libwebrtc_utility.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libaudio_coding_module.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libCNG.a \
    $$WEBRTC_BUILD/obj/webrtc/common_audio/libcommon_audio.a \
    $$WEBRTC_BUILD/obj/webrtc/system_wrappers/libsystem_wrappers.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/openmax_dl/dl/libopenmax_dl.a \
    $$WEBRTC_BUILD/obj/webrtc/common_audio/libcommon_audio_sse2.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libaudio_encoder_interface.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libG711.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libG722.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libiLBC.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libiSAC.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libaudio_decoder_interface.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libiSACFix.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libPCM16B.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libred.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libwebrtc_opus.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/opus/libopus.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libneteq.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libmedia_file.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libwebrtc_video_coding.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libwebrtc_i420.a \
    $$WEBRTC_BUILD/obj/webrtc/common_video/libcommon_video.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/video_coding/utility/libvideo_coding_utility.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/video_coding/codecs/vp8/libwebrtc_vp8.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/libvpx/libvpx.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/libvpx/libvpx_intrinsics_mmx.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/libvpx/libvpx_intrinsics_sse2.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/libvpx/libvpx_intrinsics_ssse3.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/libvpx/libvpx_intrinsics_sse4_1.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/libvpx/libvpx_intrinsics_avx2.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/video_coding/codecs/vp9/libwebrtc_vp9.a \
    $$WEBRTC_BUILD/obj/webrtc/libwebrtc.a \
    $$WEBRTC_BUILD/obj/webrtc/video_engine/libvideo_engine_core.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/librtp_rtcp.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libpaced_sender.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libremote_bitrate_estimator.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libbitrate_controller.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libvideo_capture_module.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libvideo_processing.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libvideo_processing_sse2.a \
    $$WEBRTC_BUILD/obj/webrtc/voice_engine/libvoice_engine.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libaudio_conference_mixer.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libaudio_processing.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libaudioproc_debug_proto.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/protobuf/libprotobuf_lite.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libaudio_processing_sse2.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libaudio_device.a \
    $$WEBRTC_BUILD/obj/webrtc/sound/librtc_sound.a \
    $$WEBRTC_BUILD/obj/webrtc/system_wrappers/libfield_trial_default.a \
    $$WEBRTC_BUILD/obj/webrtc/system_wrappers/libmetrics_default.a \
    $$WEBRTC_BUILD/obj/webrtc/libjingle/xmllite/librtc_xmllite.a \
    $$WEBRTC_BUILD/obj/webrtc/libjingle/xmpp/librtc_xmpp.a \
    $$WEBRTC_BUILD/obj/webrtc/p2p/librtc_p2p.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libvideo_capture_module_internal_impl.a \
    $$WEBRTC_BUILD/obj/webrtc/modules/libvideo_render_module_internal_impl.a \
    $$WEBRTC_BUILD/obj/talk/libjingle_p2p.a \
    $$WEBRTC_BUILD/obj/chromium/src/third_party/libsrtp/libsrtp.a \
    -Wl,--end-group \
    -lexpat -ldl -lrt -lXext -lX11 -lXcomposite -lXrender -lsmime3 -lnss3 \
    -lnssutil3 -lplds4 -lplc4 -lnspr4 -lm \

QMAKE_CXXFLAGS += \
    -fno-rtti

