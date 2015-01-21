/**
 * Custom message namespace for debug channel
 **/
var MESSAGE_NAMESPACE = 'urn:x-cast:ch.gorrion.pacc.mediaplayer';

var senders = {};  // a list of Chrome senders
var audioStreamIndex = -1;  // index for audio stream
var streamAudioBitrates;  // bitrates of audio stream selected

// an instance of cast.receiver.CastReceiverManager
var castReceiverManager = null;
var mediaManager = null;  // an instance of cast.receiver.MediaManager
var messageBus = null;  // custom message bus
var mediaElement = null;  // media element
var mediaHost = null;  // an instance of cast.player.api.Host
var mediaProtocol = null;  // an instance of cast.player.api.Protocol
var mediaPlayer = null;  // an instance of cast.player.api.Player

/*
 * onLoad method as entry point to initialize custom receiver
 */
onload = function() {
    mediaElement = document.getElementById('receiverAudioElement');
    mediaElement.autoplay = true;

    /**
      play – The process of play has started
      waiting – When the audio stops due to buffering
      volumechange – volume has changed
      stalled – trying to get data, but not available
      ratechange – some speed changed
      canplay – It is possible to start playback, but no guarantee of not buffering
      canplaythrough – It seems likely that we can play w/o buffering issues
      ended – the audio has finished
      error – error occured during loading of the audio
      playing – when the audio has started playing
      seeking – started seeking
      seeked – seeking has completed
     **/

    mediaElement.addEventListener('loadstart', function(e) {
        console.log('######### MEDIA ELEMENT LOAD START');
        setDebugMessage('mediaElementState', 'Load Start');
    });
    mediaElement.addEventListener('loadeddata', function(e) {
        console.log('######### MEDIA ELEMENT DATA LOADED');
        setDebugMessage('mediaElementState', 'Data Loaded');

        getPlayerState();
    });
    mediaElement.addEventListener('canplay', function(e) {
        console.log('######### MEDIA ELEMENT CAN PLAY');
        setDebugMessage('mediaElementState', 'Can Play');
        getPlayerState();
    });
    mediaElement.addEventListener('ended', function(e) {
        console.log('######### MEDIA ELEMENT ENDED');
        setDebugMessage('mediaElementState', 'Ended');
        getPlayerState();
    });
    mediaElement.addEventListener('playing', function(e) {
        console.log('######### MEDIA ELEMENT PLAYING');
        setDebugMessage('mediaElementState', 'Playing');
    });
    mediaElement.addEventListener('waiting', function(e) {
        console.log('######### MEDIA ELEMENT WAITING');
        setDebugMessage('mediaElementState', 'Waiting');
        getPlayerState();
    });
    mediaElement.addEventListener('stalled', function(e) {
        console.log('######### MEDIA ELEMENT STALLED');
        setDebugMessage('mediaElementState', 'Stalled');
        getPlayerState();
    });
    mediaElement.addEventListener('error', function(e) {
        console.log('######### MEDIA ELEMENT ERROR ' + e);
        setDebugMessage('mediaElementState', 'Error');
        getPlayerState();
    });
    mediaElement.addEventListener('abort', function(e) {
        console.log('######### MEDIA ELEMENT ABORT ' + e);
        setDebugMessage('mediaElementState', 'Abort');
        getPlayerState();
    });
    mediaElement.addEventListener('susppend', function(e) {
        console.log('######### MEDIA ELEMENT SUSPEND ' + e);
        setDebugMessage('mediaElementState', 'Suspended');
        getPlayerState();
    });
    mediaElement.addEventListener('progress', function(e) {
        setDebugMessage('mediaElementState', 'Progress');
        getPlayerState();
    });

    mediaElement.addEventListener('seeking', function(e) {
        console.log('######### MEDIA ELEMENT SEEKING ' + e);
        setDebugMessage('mediaElementState', 'Seeking');
        getPlayerState();
    });
    mediaElement.addEventListener('seeked', function(e) {
        console.log('######### MEDIA ELEMENT SEEKED ' + e);
        setDebugMessage('mediaElementState', 'Seeked');
        getPlayerState();
    });

    /**
     * Sets the log verbosity level.
     *
     * Debug logging (all messages).
     * DEBUG
     *
     * Verbose logging (sender messages).
     * VERBOSE
     *
     * Info logging (events, general logs).
     * INFO
     *
     * Error logging (errors).
     * ERROR
     *
     * No logging.
     * NONE
     **/
    cast.receiver.logger.setLevelValue(cast.receiver.LoggerLevel.DEBUG);
    cast.player.api.setLoggerLevel(cast.player.api.LoggerLevel.DEBUG);

    castReceiverManager = cast.receiver.CastReceiverManager.getInstance();

    /**
     * Called to process 'ready' event. Only called after calling
     * castReceiverManager.start(config) and the
     * system becomes ready to start receiving messages.
     * @param {cast.receiver.CastReceiverManager.Event} event - can be null
     * There is no default handler
     */
    castReceiverManager.onReady = function(event) {
        console.log('### Cast Receiver Manager is READY: ' + JSON.stringify(event));
        setDebugMessage('castReceiverManagerMessage', 'READY: ' +
                        JSON.stringify(event));
        setDebugMessage('applicationState', 'Loaded. Started. Ready.');
    };

    /**
     * If provided, it processes the 'senderconnected' event.
     * Called to process the 'senderconnected' event.
     * @param {cast.receiver.CastReceiverManager.Event} event - can be null
     *
     * There is no default handler
     */
    castReceiverManager.onSenderConnected = function(event) {
        console.log('### Cast Receiver Manager - Sender Connected : ' +
                    JSON.stringify(event));
        setDebugMessage('castReceiverManagerMessage', 'Sender Connected: ' +
                        JSON.stringify(event));

        senders = castReceiverManager.getSenders();
        setDebugMessage('senderCount', '' + senders.length);
    };

    /**
     * If provided, it processes the 'senderdisconnected' event.
     * Called to process the 'senderdisconnected' event.
     * @param {cast.receiver.CastReceiverManager.Event} event - can be null
     *
     * There is no default handler
     */
    castReceiverManager.onSenderDisconnected = function(event) {
        console.log('### Cast Receiver Manager - Sender Disconnected : ' +
                    JSON.stringify(event));
        setDebugMessage('castReceiverManagerMessage', 'Sender Disconnected: ' +
                        JSON.stringify(event));

        senders = castReceiverManager.getSenders();
        setDebugMessage('senderCount', '' + senders.length);
    };

    /**
     * If provided, it processes the 'systemvolumechanged' event.
     * Called to process the 'systemvolumechanged' event.
     * @param {cast.receiver.CastReceiverManager.Event} event - can be null
     *
     * There is no default handler
     */
    castReceiverManager.onSystemVolumeChanged = function(event) {
        console.log('### Cast Receiver Manager - System Volume Changed : ' +
                    JSON.stringify(event));
        setDebugMessage('castReceiverManagerMessage', 'System Volume Changed: ' +
                        JSON.stringify(event));

        // See cast.receiver.media.Volume
        console.log('### Volume: ' + event.data['level'] + ' is muted? ' +
                    event.data['muted']);
        setDebugMessage('volumeMessage', 'Level: ' + event.data['level'] +
                        ' -- muted? ' + event.data['muted']);
    };

    /**
     * Use the messageBus to listen for incoming messages on a virtual channel
     * using a namespace string.Also use messageBus to send messages back to a
     * sender or broadcast a message to all senders.
     *
     * You can check the cast.receiver.CastMessageBus.MessageType that a message
     * bus processes though a call to getMessageType. As well, you get the
     * namespace of a message bus by calling getNamespace()
     */
    messageBus = castReceiverManager.getCastMessageBus(MESSAGE_NAMESPACE);

    /**
     * This message bus is used to identify the protocol of showing/hiding the 
     * heads up display messages
     * (The messages defined at the beginning of the html).
     *
     * The protocol consists of one string message: show
     * In the case of the message value not being show - the assumed value is hide.
     * @param {Object} event A returned object from callback
     **/
    messageBus.onMessage = function(event) {
        console.log('### Message Bus - Media Message: ' + JSON.stringify(event));
        setDebugMessage('messageBusMessage', event);

        console.log('### CUSTOM MESSAGE: ' + JSON.stringify(event));
        // show/hide messages
        console.log(event['data']);
        var payload = JSON.parse(event['data']);
        if (payload['type'] === 'show') {
            if (payload['target'] === 'debug') {
                document.getElementById('messages').style.display = 'block';
            } else {
                document.getElementById('receiverAudioElement').style.display = 'block';
            }
        } else if (payload['type'] === 'hide') {
            if (payload['target'] === 'debug') {
                document.getElementById('messages').style.display = 'none';
            } else {
                document.getElementById('receiverAudioElement').style.display = 'none';
            }
        }
        broadcast(event['data']);
    };


    mediaManager = new cast.receiver.MediaManager(mediaElement);

    /**
     * Called when the media ends.
     *
     * mediaManager.resetMediaElement(cast.receiver.media.IdleReason.FINISHED);
     **/
    mediaManager['onEndedOrig'] = mediaManager.onEnded;
    /**
     * Called when the media ends
     */
    mediaManager.onEnded = function() {
        setDebugMessage('mediaManagerMessage', 'ENDED');

        mediaManager['onEndedOrig']();
    };

    /**
     * Default implementation of onError.
     *
     * mediaManager.resetMediaElement(cast.receiver.media.IdleReason.ERROR)
     **/
    mediaManager['onErrorOrig'] = mediaManager.onError;
    /**
     * Called when there is an error not triggered by a LOAD request
     * @param {Object} obj An error object from callback
     */
    mediaManager.onError = function(obj) {
        setDebugMessage('mediaManagerMessage', 'ERROR - ' + JSON.stringify(obj));

        mediaManager['onErrorOrig'](obj);
        if (mediaPlayer) {
            mediaPlayer.unload();
            mediaPlayer = null;
        }
    };

    /**
     * Processes the get status event.
     *
     * Sends a media status message to the requesting sender (event.data.requestId)
     **/
    mediaManager['onGetStatusOrig'] = mediaManager.onGetStatus;
    /**
     * Processes the get status event.
     * @param {Object} event An status object
     */
    mediaManager.onGetStatus = function(event) {
        console.log('### Media Manager - GET STATUS: ' + JSON.stringify(event));
        setDebugMessage('mediaManagerMessage', 'GET STATUS ' +
                        JSON.stringify(event));

        mediaManager['onGetStatusOrig'](event);
    };

    /**
     * Default implementation of onLoadMetadataError.
     *
     * mediaManager.resetMediaElement(cast.receiver.media.IdleReason.ERROR, false);
     * mediaManager.sendLoadError(cast.receiver.media.ErrorType.LOAD_FAILED);
     **/
    mediaManager['onLoadMetadataErrorOrig'] = mediaManager.onLoadMetadataError;
    /**
     * Called when load has had an error, overridden to handle application
     * specific logic.
     * @param {Object} event An object from callback
     */
    mediaManager.onLoadMetadataError = function(event) {
        console.log('### Media Manager - LOAD METADATA ERROR: ' +
                    JSON.stringify(event));
        setDebugMessage('mediaManagerMessage', 'LOAD METADATA ERROR: ' +
                        JSON.stringify(event));

        mediaManager['onLoadMetadataErrorOrig'](event);
    };

    /**
     * Default implementation of onMetadataLoaded
     *
     * Passed a cast.receiver.MediaManager.LoadInfo event object
     * Sets the mediaElement.currentTime = loadInfo.message.currentTime
     * Sends the new status after a LOAD message has been completed succesfully.
     * Note: Applications do not normally need to call this API.
     * When the application overrides onLoad, it may need to manually declare that
     * the LOAD request was sucessful. The default implementaion will send the new
     * status to the sender when the audio element raises the
     * 'loadedmetadata' event.
     * The default behavior may not be acceptable in a couple scenarios:
     *
     * 1) When the application does not want to declare LOAD succesful until for
     *    example 'canPlay' is raised (instead of 'loadedmetadata').
     * 2) When the application is not actually loading the media element (for
     *    example if LOAD is used to load an image).
     **/
    mediaManager['onLoadMetadataOrig'] = mediaManager.onLoadMetadataLoaded;
    /**
     * Called when load has completed, overridden to handle application specific
     * logic.
     * @param {Object} event An object from callback
     */
    mediaManager.onLoadMetadataLoaded = function(event) {
        console.log('### Media Manager - LOADED METADATA: ' +
                    JSON.stringify(event));
        setDebugMessage('mediaManagerMessage', 'LOADED METADATA: ' +
                        JSON.stringify(event));
        mediaManager['onLoadMetadataOrig'](event);
    };

    /**
     * Processes the pause event.
     *
     * mediaElement.pause();
     * Broadcast (without sending media information) to all senders that pause has
     * happened.
     **/
    mediaManager['onPauseOrig'] = mediaManager.onPause;
    /**
     * Process pause event
     * @param {Object} event
     */
    mediaManager.onPause = function(event) {
        console.log('### Media Manager - PAUSE: ' + JSON.stringify(event));
        setDebugMessage('mediaManagerMessage', 'PAUSE: ' + JSON.stringify(event));
        mediaManager['onPauseOrig'](event);
    };

    /**
     * Default - Processes the play event.
     *
     * mediaElement.play();
     *
     **/
    mediaManager['onPlayOrig'] = mediaManager.onPlay;
    /**
     * Process play event
     * @param {Object} event
     */
    mediaManager.onPlay = function(event) {
        console.log('### Media Manager - PLAY: ' + JSON.stringify(event));
        setDebugMessage('mediaManagerMessage', 'PLAY: ' + JSON.stringify(event));

        mediaManager['onPlayOrig'](event);
    };

    /**
     * Default implementation of the set volume event.
     * Checks event.data.volume.level is defined and sets the mediaElement.volume
     * to the value.
     * Checks event.data.volume.muted is defined and sets the mediaElement.muted
     * to the value.
     * Broadcasts (without sending media information) to all senders that the
     * volume has changed.
     **/
    mediaManager['onSetVolumeOrig'] = mediaManager.onSetVolume;
    /**
     * Process set volume event
     * @param {Object} event
     */
    mediaManager.onSetVolume = function(event) {
        console.log('### Media Manager - SET VOLUME: ' + JSON.stringify(event));
        setDebugMessage('mediaManagerMessage', 'SET VOLUME: ' +
                        JSON.stringify(event));

        mediaManager['onSetVolumeOrig'](event);
    };

    /**
     * Processes the stop event.
     *
     * mediaManager.resetMediaElement(cast.receiver.media.IdleReason.CANCELLED,
     *   true, event.data.requestId);
     *
     * Resets Media Element to IDLE state. After this call the mediaElement
     * properties will change, paused will be true, currentTime will be zero and
     * the src attribute will be empty. This only needs to be manually called if
     * the developer wants to override the default behavior of onError, onStop or
     * onEnded, for example.
     **/
    mediaManager['onStopOrig'] = mediaManager.onStop;
    /**
     * Process stop event
     * @param {Object} event
     */
    mediaManager.onStop = function(event) {
        console.log('### Media Manager - STOP: ' + JSON.stringify(event));
        setDebugMessage('mediaManagerMessage', 'STOP: ' + JSON.stringify(event));

        mediaManager['onStopOrig'](event);
    };

    /**
     * Default implementation for the load event.
     *
     * Sets the mediaElement.autoplay to false.
     * Checks that data.media and data.media.contentId are valid then sets the
     * mediaElement.src to the data.media.contentId.
     *
     * Checks the data.autoplay value:
     *   - if undefined sets mediaElement.autoplay = true
     *   - if has value then sets mediaElement.autoplay to that value
     **/
    mediaManager['onLoadOrig'] = mediaManager.onLoad;
    /**
     * Processes the load event.
     * @param {Object} event
     */
    mediaManager.onLoad = function(event) {
        console.log('### Media Manager - LOAD: ' + JSON.stringify(event));
        setDebugMessage('mediaManagerMessage', 'LOAD ' + JSON.stringify(event));

        if (mediaPlayer !== null) {
            mediaPlayer.unload(); // Ensure unload before loading again
        }

        if (event.data['media'] && event.data['media']['contentId']) {
            var url = event.data['media']['contentId'];

            setDebugMessage('mediaPlayerState', '-');

            mediaHost = new cast.player.api.Host({
                'mediaElement': mediaElement,
                'url': url
            });

            var customData = event.data['customData'];
            var streamSecret = customData['stream_secret'];
            mediaHost.updateManifestRequestInfo = function(requestInfo) {
                if (!requestInfo.url) {
                    requestInfo.url = url;
                }

                requestInfo.headers = {}
                requestInfo.headers["Authorization"] = streamSecret;
            };
            
            mediaHost.updateSegmentRequestInfo = function(requestInfo) {
                requestInfo.headers = {}
                requestInfo.headers["Authorization"] = streamSecret;
            };

            mediaHost.onError = function(errorCode, requestStatus) {
                console.error('### HOST ERROR - Fatal Error: code = ' + errorCode);
                setDebugMessage('mediaHostState', 'Fatal Error: code = ' + errorCode);
                if (mediaPlayer !== null) {
                    mediaPlayer.unload();
                }
            };

            // Advanced Playback - HLS
            // Player registers to listen to the media element events through the
            // mediaHost property of the  mediaElement
            protocol = cast.player.api.CreateHlsStreamingProtocol(mediaHost, cast.player.api.HlsSegmentFormat.MPEG_AUDIO_ES);
            mediaPlayer = new cast.player.api.Player(mediaHost);
            mediaPlayer.load(protocol, Infinity);

            setDebugMessage('mediaHostState', 'success');
        }
    };

    console.log('### Application Loaded. Starting system.');
    setDebugMessage('applicationState', 'Loaded. Starting up.');

    /**
     * Application config
     **/
    var appConfig = new cast.receiver.CastReceiverManager.Config();

    /**
     * Text that represents the application status. It should meet
     * internationalization rules as may be displayed by the sender application.
     * @type {string|undefined}
     **/
    appConfig.statusText = 'Ready to play';

    /**
     * Maximum time in seconds before closing an idle
     * sender connection. Setting this value enables a heartbeat message to keep
     * the connection alive. Used to detect unresponsive senders faster than
     * typical TCP timeouts. The minimum value is 5 seconds, there is no upper
     * bound enforced but practically it's minutes before platform TCP timeouts
     * come into play. Default value is 10 seconds.
     * @type {number|undefined}
     * 10 minutes for testing, use default 10sec in prod by not setting this value
     **/
    appConfig.maxInactivity = 6000;

    /**
     * Initializes the system manager. The application should call this method when
     * it is ready to start receiving messages, typically after registering
     * to listen for the events it is interested on.
     */
    castReceiverManager.start(appConfig);
};

/*
 * send message to a sender via custom message channel
 @param {string} senderId A id string for specific sender
 @param {string} message A message string
 */
function messageSender(senderId, message) {
    messageBus.send(senderId, message);
}

/*
 * broadcast message to all senders via custom message channel
 @param {string} message A message string
 */
function broadcast(message) {
    messageBus.broadcast(message);
}

/*
 * set debug message on receiver screen/TV
 @param {string} message A message string
 */
function setDebugMessage(elementId, message) {
    document.getElementById(elementId).innerHTML = '' + JSON.stringify(message);
}

/*
 * get media player state
 */
function getPlayerState() {
    var playerState = mediaPlayer.getState();
    setDebugMessage('mediaPlayerState', 'underflow: ' + playerState['underflow']);
}
