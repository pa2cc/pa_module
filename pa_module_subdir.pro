TEMPLATE = subdirs

SUBDIRS += pa_module webrtc

pa_module.file = pa_module.pro
pa_module.depends = webrtc

webrtc.subdir = webrtc
