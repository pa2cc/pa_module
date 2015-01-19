/**
 * CONSTANTS
 */
/**
 * Cast initialization timer delay
 **/
var CAST_API_INITIALIZATION_DELAY = 1000;

/**
 * Custom message namespace for debug channel
 **/
var MESSAGE_NAMESPACE = 'urn:x-cast:ch.gorrion.pacc.mediaplayer';

/**
 * Media constant: media types
 */
var MEDIA_TYPE = 'application/vnd.apple.mpegurl';

/**
 * Application ID
 */
var APPLICATION_ID = '9FF06292';

/**
 * The url of the control server.
 */
var CONTROL_URL = 'http://localhost:51348';

/**
 * global variables
 */

/**
 * The stream info.
 */
var streamInfo = null;

/**
 * The volume listener.
 */
var volumeListener = null;

/**
 * * Current media session
 * */
var currentMedia = null;

/**
 * Current media volume level
 */
var currentVolume = 0.5;

/**
 * Current session object
 */
var session = null;

/**
 * Call Cast API initialization
 */
if (!chrome.cast || !chrome.cast.isAvailable) {
    setTimeout(initializeCastApi, CAST_API_INITIALIZATION_DELAY);
}

/**
 * Initialization
 */
function initializeCastApi() {
    var sessionRequest = new chrome.cast.SessionRequest(APPLICATION_ID);
    var apiConfig = new chrome.cast.ApiConfig(sessionRequest,
                                              sessionListener,
                                              receiverListener);

    chrome.cast.initialize(apiConfig, onInitSuccess, onError);
}

/**
 * initialization success callback
 */
function onInitSuccess() {
    console.log('init success');
}

/**
 * initialization error callback
 */
function onError() {
    console.log('error');
}

/**
 * generic success callback
 * @param {string} message A message string
 */
function onSuccess(message) {
  console.log(message);
}

/**
 * callback on success for stopping app
 */
function onStopAppSuccess() {
    console.log('Session stopped');
}

/**
 * session listener during initialization
 * @param {Object} e A session object
 * @this sesssionListener
 */
function sessionListener(e) {
    console.log('New session ID: ' + e.sessionId);
    session = e;

    // Stops all running media.
    for (var i = 0; i < session.media.length; ++i) {
        session.media[i].stop(
            null,
            mediaCommandSuccessCallback.bind(this, 'stopped ' + session.media[i].sessionId),
            onError);
    }

    loadStreamInfo();

    session.addMediaListener(onMediaDiscovered.bind(this, 'addMediaListener'));
    session.addUpdateListener(sessionUpdateListener.bind(this));
    session.addMessageListener(MESSAGE_NAMESPACE, onReceiverMessage.bind(this));
}


/**
 * handle message from receiver app
 * @param {string} namespace A message string
 * @param {string} message A message string
 */
function onReceiverMessage(namespace, message) {
    var messageJSON = JSON.parse(message);
    console.log(namespace + ':' + message);
}

/**
 * @param {boolean} isAlive A boolean
 * true for session being live
 */
function sessionUpdateListener(isAlive) {
    var message = isAlive ? 'Session Updated' : 'Session Removed';
    message += ': ' + session.sessionId;
    console.log(message);

    if (!isAlive) {
        session = null;
        var playpauseresume = document.getElementById('playpauseresume');
        playpauseresume.innerHTML = 'Play Media';
    }
}

/**
 * receiver listener during initialization
 * @param {string} e A message string
 */
function receiverListener(e) {
    if (e === 'available') {
        console.log('receiver found');
        setTimeout(function() {
            if (!session) {
                launchApp();
            }}, 1000);
    } else {
        console.log('receiver list empty');
    }
}

/**
 * launch app
 */
function launchApp() {
    console.log('launching app...');
    chrome.cast.requestSession(onRequestSessionSuccess, onLaunchError);
}

/**
 * callback on success for requestSession call
 * @param {Object} e A non-null new session.
 * @this onRequestSessionSuccess
 */
function onRequestSessionSuccess(e) {
    console.log('session success: ' + e.sessionId);
    session = e;
    session.addMessageListener(MESSAGE_NAMESPACE, onReceiverMessage.bind(this));
}

/**
 * callback on launch error
 */
function onLaunchError() {
    console.log('launch error');
}

/**
 * stop app/session
 */
function stopApp() {
    if (session != null) {
        session.stop(onStopAppSuccess, onError);
        return;
    }
}

/**
 * Loads the stream info from the control channel.
 * @this loadStreamInfo
 */
function loadStreamInfo() {
    var xhr = new XMLHttpRequest();
    xhr.open("GET", CONTROL_URL + "/streamInfo", true);
    xhr.onload = function(ev) {
        if (this.status != 200) {
            console.log('Error while getting the stream info.');
            return;
        }

        streamInfo = JSON.parse(this.responseText);

        if (!volumeListener) {
            volumeListener = new VolumeListener();
            volumeListener.start();
        }

        loadMedia();
    };
    xhr.onerror = function(ev) {
        console.log('Could not get the stream info!');
    };
    xhr.send();
}

/**
 * load media
 * @this loadMedia
 */
function loadMedia() {
    if (!session) {
        console.log('no session');
        return;
    }
    if (!streamInfo) {
        console.log('no stream info');
        return;
    }

    var mediaInfo =
        new chrome.cast.media.MediaInfo(streamInfo['stream_urls'][0]);
    mediaInfo.contentType = MEDIA_TYPE;

    var request = new chrome.cast.media.LoadRequest(mediaInfo);
    request.currentTime = Infinity;
    request.autoplay = true;
    request.media.streamType = chrome.cast.media.StreamType.LIVE; // by sämy

    request.customData = {
        'stream_secret': streamInfo['stream_secret']
    };

    session.loadMedia(request,
                      onMediaDiscovered.bind(this, 'loadMedia'),
                      onMediaError);
}

/**
 * callback on success for loading media
 * @param {string} how A message string from callback
 * @param {Object} mediaSession A media session object
 */
function onMediaDiscovered(how, mediaSession) {
    console.log('new media session ID:' + mediaSession.mediaSessionId);

    currentMedia = mediaSession;
    mediaSession.addUpdateListener(onMediaStatusUpdate);
    mediaCurrentTime = currentMedia.currentTime;
    playpauseresume.innerHTML = 'Pause Media';
}

/**
 * callback on media loading error
 * @param {Object} e A non-null media object
 */
function onMediaError(e) {
    console.log('media error');
}

/**
 * callback for media status event
 * @param {string} isAlive A string from callback
 */
function onMediaStatusUpdate(isAlive) {
    document.getElementById('playerstate').innerHTML = currentMedia.playerState;
}

/**
 * play media
 * @this playMedia
 */
function playMedia() {
    if (!currentMedia) {
        return;
    }

    var playpauseresume = document.getElementById('playpauseresume');
    if (playpauseresume.innerHTML == 'Play Media') {
        currentMedia.play(null, mediaCommandSuccessCallback.bind(
                this, 'playing ' + currentMedia.sessionId), onError);
        playpauseresume.innerHTML = 'Pause Media';
        currentMedia.addUpdateListener(onMediaStatusUpdate);
        console.log('play started');
    } else {
        if (playpauseresume.innerHTML == 'Pause Media') {
            currentMedia.pause(null, mediaCommandSuccessCallback.bind(
                    this, 'paused ' + currentMedia.sessionId), onError);
            playpauseresume.innerHTML = 'Resume Media';
            console.log('paused');
        } else {
            if (playpauseresume.innerHTML == 'Resume Media') {
                currentMedia.play(null, mediaCommandSuccessCallback.bind(
                        this, 'resumed ' + currentMedia.sessionId), onError);
                playpauseresume.innerHTML = 'Pause Media';
                console.log('resumed');
            }
        }
    }
}

/**
 * stop media
 * @this stopMedia
 */
function stopMedia() {
    if (!currentMedia) {
        return;
    }

    currentMedia.stop(null,
                      mediaCommandSuccessCallback.bind(this, 'stopped ' + currentMedia.sessionId),
                      onError);
    var playpauseresume = document.getElementById('playpauseresume');
    playpauseresume.innerHTML = 'Play Media';
    console.log('media stopped');
}

/**
 * set receiver volume
 * @param {Number} level A number for volume level
 * @param {Boolean} mute A true/false for mute/unmute
 * @this setReceiverVolume
 */
function setReceiverVolume(level, mute) {
    if (!session) {
        return;
    }

    if (!mute) {
        session.setReceiverVolumeLevel(level,
                                       mediaCommandSuccessCallback.bind(this, 'media set-volume done'),
                                       onError);
        currentVolume = level;
    } else {
        session.setReceiverMuted(true,
                                 mediaCommandSuccessCallback.bind(this, 'media set-volume done'),
                                 onError);
    }
}

/**
 * callback on success for media commands
 * @param {string} info A message string
 */
function mediaCommandSuccessCallback(info) {
    console.log(info);
}

/**
 * send a custom message to receiver so that
 * it will display debug message on TV
 */
function showReceiverDebugMessage() {
    sendMessage({'type': 'show', 'target': 'debug'});
}

/**
 * send a custom message to receiver so that
 * it will hide debug message on TV
 */
function hideReceiverDebugMessage() {
    sendMessage({'type': 'hide', 'target': 'debug'});
}

/**
 * @param {string} message A message string
 * @this sendMessage
 */
function sendMessage(message) {
    if (session != null) {
        session.sendMessage(MESSAGE_NAMESPACE, message,
                            onSuccess.bind(this, 'Message sent: ' + message),
                            onError);
    }
}
