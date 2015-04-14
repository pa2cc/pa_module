# PACC

PACC enables you to use your Chromecast as an output device in Pulseaudio. This 
allows you to play music from your favorite player directly to your TV.

## How to use
Using PACC after installing the Pulseaudio module is very easy. Point Chromium
to https://www.gorrion.ch/pacc/app/ and start casting. The only thing left is
specifying `PACC` as the output device of your music player. Enjoy.

## How to install
Currently, we do not have any pre-compiled versions available (coming soon).
Therefore, you need to build PACC yourself:

1. Download the PACC source to `[pacc_root]`.
2. Get the Pulseaudio source from
   http://freedesktop.org/software/pulseaudio/releases into
   `[pacc_root]/third_party/pulseaudio-6.0/`.
    * Check how your distro built the library - do a call to `./configure`.
3. Check out http://www.webrtc.org/native-code/development for how to get and
   build the WebRTC source.
    * Call `fetch webrtc` in `[pacc_root]/third_party/webrtc/`.
4. Generate your local PACC certificate.
    * `make -C [pacc_root]/pa_module/res certs`
5. Build the Pulseaudio module (in `[pacc_root]/pa_module`)
    * `qmake pa_module_subdir.pro`
    * `make`
    * `mv libpa_module.so /usr/lib/pulse-6.0/modules/module-pacc-sink.so`
6. Ensure that the PACC Pulseaudio module is automatically loaded.
    * `echo "load-module module-pacc-sink" >> /etc/pulse/default.pa`
    * `pactl load-module module-pacc-sink`

