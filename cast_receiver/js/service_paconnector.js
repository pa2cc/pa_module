'use strict';

var paccServices = angular.module('paccServices');

paccServices.service('PAConnector', ['$rootScope',
function($rootScope) {
    // CONSTANTS
    var self = this;
    
    /**
     * Message namespace for the message channel.
     **/
    var MESSAGE_NAMESPACE = 'urn:x-cast:ch.gorrion.pacc';
    
    var MSG_TYPE_RESET = 'reset';
    var MSG_TYPE_GET_ICE_CANDIDATES = 'getIceCandidates';
    var MSG_TYPE_GET_SESSION_DESCRIPTION = 'getSessionDescription';
    var MSG_TYPE_ICE_CANDIDATE = 'iceCandidate';
    var MSG_TYPE_SESSION_DESCRIPTION = 'sessionDescription';
    
    // Variables

    /**
     * The cast.receiver.CastReceiverManager instance.
     */
    var castReceiverManager = null;
    
    /**
     * The message bus to the chrome apps.
     */
    var messageBus = null;

    
    // Public API.
    this.sendCCIceCandidate = function(ccIceCandidate) {
        sendMessage(MSG_TYPE_ICE_CANDIDATE, ccIceCandidate);
    }
    this.sendCCSessionDescription = function(ccSessionDescription) {
        sendMessage(MSG_TYPE_SESSION_DESCRIPTION, ccSessionDescription);
    }
    this.getIceCandidates = function() {
        sendMessage(MSG_TYPE_GET_ICE_CANDIDATES);
    }
    this.getSessionDescription = function() {
        sendMessage(MSG_TYPE_GET_SESSION_DESCRIPTION);
    }
    
    
    // Initialization.

    // Sets the log verbosity level.
    cast.receiver.logger.setLevelValue(cast.receiver.LoggerLevel.DEBUG);
    cast.player.api.setLoggerLevel(cast.player.api.LoggerLevel.DEBUG);

    // Initializes the cast receiver manager.
    castReceiverManager = cast.receiver.CastReceiverManager.getInstance();

    // Initializes the messageBus to communicate with the chrome app.
    messageBus = castReceiverManager.getCastMessageBus(
        MESSAGE_NAMESPACE, cast.receiver.CastMessageBus.MessageType.JSON);

    /**
     * Callback that gets called when a message arrives at the message bus.
     * @param {Object} event A returned object from callback
     **/
    messageBus.onMessage = function(event) {
        var message = event.data;
        if (MSG_TYPE_ICE_CANDIDATE === message.type) {
            $rootScope.$broadcast('PAConnector:onPAIceCandidate', message.data);
        } else if (MSG_TYPE_SESSION_DESCRIPTION === message.type) {
            $rootScope.$broadcast('PAConnector:onPASessionDescription',
                                  message.data);
        } else {
            console.warn('Unknown message type: ' + message.type);
        }
    };
    
    /**
     * Sends the given message to the chrome app.
     * @param {string} type The message type
     * @param {Object} data The payload
     */
    function sendMessage(type, data) {
        messageBus.broadcast({
            type: type,
            data: data
        });
    }
    
    // Initializes the application configuration.
    var appConfig = new cast.receiver.CastReceiverManager.Config();
    appConfig.maxInactivity = 6000;

    // Initializes the system manager.
    castReceiverManager.start(appConfig);
    
    
    var sessionDescriptionRequestIssued = false;
    castReceiverManager.onSenderConnected = function(event) {
        // We request the session description from it.
        if (!sessionDescriptionRequestIssued) {
            sessionDescriptionRequestIssued = true;
        
            // We request the pulseaudio module to reset.
            sendMessage(MSG_TYPE_RESET);
            
            // And then to give us it's session description.
            self.getSessionDescription();
        }
    };

}]).run(function(PAConnector) {}); // Ensures that the service is loaded.
