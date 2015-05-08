'use strict';

var RTCPeerConnection = window.RTCPeerConnection || window.mozRTCPeerConnection || window.webkitRTCPeerConnection;
var RTCSessionDescription = window.RTCSessionDescription || window.mozRTCSessionDescription || window.webkitRTCSessionDescription;
var RTCIceCandidate = window.RTCIceCandidate || window.mozRTCIceCandidate || window.webkitRTCIceCandidate;

var paccServices = angular.module('paccServices');

paccServices.service('WebRTC', ['$rootScope', 'PAConnector',
function($rootScope, PAConnector) {
    // CONSTANTS
    var self = this;
    
    var STUN_SERVER = 'stun:stun.l.google.com:19302';

   
    // Public API.
    this.audio_url = null;
    
    
    // Initialization.
    
    // Creates the PeerConnection.
    var pcConfiguration = {
        iceServers: [{url: STUN_SERVER}],
    };
    var pcOptions = {
        optional: [{DtlsSrtpKeyAgreement: true}],
    };
    var pc = new RTCPeerConnection(pcConfiguration, pcOptions);

    // Broadcasts the ice candidates once they pop up.
    pc.onicecandidate = function(event) {
        if (event.candidate) {
            PAConnector.sendCCIceCandidate(event.candidate);
        }
    };
    
    // Once the audio stream is received from the WebRTC channel we trigger an
    // angularjs update.
    pc.onaddstream = function(event) {
        $rootScope.$apply(function() {
            self.audio_url = URL.createObjectURL(event.stream);
        });
    };

    /**
     * Generic error callback.
     */
    function onError(where, e) {
        console.error('[' + where + ']: ' + e);
    }


    // Event listeners.
    $rootScope.$on('PAConnector:onPASessionDescription', function(event, paSessionDescription) {
        if (pc.remoteDescription) {
            return;
        }
        
        pc.setRemoteDescription(new RTCSessionDescription(paSessionDescription),
            function() {
                // We ask for the ICE candidates (they are delivered
                // asynchronously).
                PAConnector.getIceCandidates();
                
                // Creates the anser.
                pc.createAnswer(function(ccSessionDescription) {
                    pc.setLocalDescription(ccSessionDescription, function() {
                        PAConnector.sendCCSessionDescription(ccSessionDescription);
                    }, onError.bind(undefined, 'setLocalDescriptor'));
                }, onError.bind(undefined, 'createAnswer'));
            }, onError.bind(undefined, 'setRemoteDescritption'));
    });
    
    $rootScope.$on('PAConnector:onPAIceCandidate', function(event, paIceCandidate) {
        pc.addIceCandidate(new RTCIceCandidate(paIceCandidate),
                           function() {},
                           onError.bind(undefined, 'addIceCandidate'));
    });
}]).run(function(WebRTC) {}); // Ensures that the service is loaded.
