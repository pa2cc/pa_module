# PACC

PACC enables you to use your Chromecast as an output device in Pulseaudio. This 
allows you to play music from your favorite player directly to your TV.

This is the PulseAudio module. It creates a virtual loudspeaker where you then
can send all your music to.


## How to use
First, you need to install the PulseAudio module. This has to be done only once
and is described in the next section. Now, you can open your PulseAudio settings
and choose the "PACC bridge" as your output device. Finally, you need to point
Chrome to <https://pa2cc.github.io/cast_receiver/> and start casting to your
Chromecast. Enjoy!


## How to install
Currently, we do not have any pre-compiled versions available. Therefore, you
need to build PACC by yourself:

1. Download the PACC source to `$PACC_ROOT`.
2. Get the PulseAudio source from
   <http://freedesktop.org/software/pulseaudio/releases> into
   `$PACC_ROOT/third_party/pulseaudio-X.Y/`.
    * Check how your distro built the library.
    * `cd $PACC_ROOT/third_party/pulseaudio-X.Y/`
    * `./configure [...]`
3. Check out <http://www.webrtc.org/native-code/development> for how to get and
   build the WebRTC source.
    * `cd $PACC_ROOT/third_party/webrtc/`
    * `fetch webrtc`
4. Generate your local PACC certificate.
    * `make -C $PACC_ROOT/pa_module/res certs`
5. Build the PulseAudio module
    * `cd $PACC_ROOT/pa_module/`
    * `qmake pa_module_subdir.pro`
    * `make`
    * `sudo mv libpa_module.so /usr/lib/pulse-X.Y/modules/module-pacc-sink.so`
6. Ensure that the PACC PulseAudio module is automatically loaded.
    * `sudo echo "load-module module-pacc-sink" >> /etc/pulse/default.pa`
    * `sudo pactl load-module module-pacc-sink`

