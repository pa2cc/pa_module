'use strict';

var paccServices = angular.module('paccServices');

paccServices.service('CCConnector', ['$rootScope', 'PAConnector',
function($rootScope, PAConnector){
    // CONSTANTS
    
    /**
     * Cast initialization timer delay.
     **/
    var CAST_API_INITIALIZATION_DELAY = 1000;

    /**
     * Message namespace for the message channel.
     **/
    var MESSAGE_NAMESPACE = 'urn:x-cast:ch.gorrion.pacc';

    /**
     * Application ID
     */
    var APPLICATION_ID = '9FF06292';


    // Global variables

    var self = this;

    /**
     * Current session object.
     */
    var session = null;

    /**
     * Messages that are to be delivered as soon as a session is opened.
     */
    var pendingMessages = [];


    // Public API.
    
    self.connected = false;

    /**
     * Sends a message to receiver so that it will display the debug message on
     * TV.
     */
    self.showReceiverDebugMessage = function() {
        sendMessage('show', {'target': 'debug'});
    }

    /**
     * Send a message to receiver so that it will hide the debug message on TV.
     */
    self.hideReceiverDebugMessage = function() {
        sendMessage('hide', {'target': 'debug'});
    }


    // Event listeners.

    $rootScope.$on('PAConnector:onMessage', function(event, message) {
        sendMessage(message);
    });

    
    // Functions.
    /**
     * Generic success callback.
     * @param {string} message A message string
     */
    function onSuccess(message) {
      console.log(message);
    }

    /**
     * Generic error callback.
     */
    function onError(where, e) {
        console.error('[' + where + ']' + e);
    }

    /**
     * Initializes the cast API.
     */
    function initializeCastApi() {
        var sessionRequest = new chrome.cast.SessionRequest(APPLICATION_ID);
        var apiConfig = new chrome.cast.ApiConfig(
            sessionRequest, onSession, onReceiver,
            chrome.cast.AutoJoinPolicy.ORIGIN_SCOPED);

        chrome.cast.initialize(apiConfig, onInitSuccess, onError.bind(undefined, 'initialize'));
    }

    /**
     * Initialization success callback.
     */
    function onInitSuccess() {
        console.log('init success');
    }

    /**
     * Callback on success for stopping app.
     */
    function onStopAppSuccess() {
        console.log('Session stopped');
    }

    /**
     * Session listener callback. Gets called when a session is connected.
     * @param {chrome.cast.Session} s A session object
     */
    function onSession(s) {
        $rootScope.$apply(function() {
            self.connected = true;
        });

        console.log('New session ID: ' + s.sessionId);
        session = s;

        session.addUpdateListener(onSessionUpdate);
        session.addMessageListener(MESSAGE_NAMESPACE, onReceiveMessage);
        
        sendPendingMessages();
    }


    /**
     * Message callback. Gets called when the chromecast sent a message to us.
     * @param {string} namespace A message string
     * @param {string} message A message string
     */
    function onReceiveMessage(namespace, message) {
        if (namespace != MESSAGE_NAMESPACE) {
            return;
        }
        
        message = JSON.parse(message);
        PAConnector.send(message);
    }

    /**
     * Session update callback. Gets called when the session has changed.
     * @param {boolean} isAlive Flag if the session is being live.
     */
    function onSessionUpdate(isAlive) {
        var message = isAlive ? 'Session Updated' : 'Session Removed';
        message += ': ' + session.sessionId;
        console.log(message);

        if (!isAlive) {
            $rootScope.$apply(function() {
                self.connected = false;
            });
            session = null;
            var playpauseresume = document.getElementById('playpauseresume');
            playpauseresume.innerHTML = 'Play Media';
        }
    }

    /**
     * Receiver listener callback. Gets called when a receiver is found.
     * @param {chrome.cast.ReceiverAvailability} availability 
     *          The availability of the receiver.
     */
    function onReceiver(availability) {
        if (availability === chrome.cast.ReceiverAvailability.AVAILABLE) {
            console.log('receiver found');
            setTimeout(function() {
                if (!session) {
                    launchApp();
                }}, 1000);
        }
    }

    /**
     * Launches the chromecast app by issuing a session request.
     */
    function launchApp() {
        console.log('launching app...');
        chrome.cast.requestSession(onSession, onError.bind(undefined, 'requestSession'));
    }

    /**
     * Sends the given message to the chromecast.
     * @param {Object} message The message
     */
    function sendMessage(message) {
        if (session != null) {
            session.sendMessage(MESSAGE_NAMESPACE, message,
                                function() {},
                                onError.bind(undefined, 'sendMessage'));
        } else {
            pendingMessages.push(message);
        }
    }
    
    /**
     * Sends all pending messages.
     */
    function sendPendingMessages() {
        while (pendingMessages.length > 0) {
            var message = pendingMessages.pop();
            sendMessage(message);
        }
    }

    
    // Initialization code.

    if (!chrome.cast || !chrome.cast.isAvailable) {
        setTimeout(initializeCastApi, CAST_API_INITIALIZATION_DELAY);
    }
}]).run(function(CCConnector) {}); // Ensures that the service is loaded.
