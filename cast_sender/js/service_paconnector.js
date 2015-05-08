'use strict';

var paccServices = angular.module('paccServices');

/**
 * Service that enables the communication with the  pulseaudio module by opening
 * a websocket to it.
 */
paccServices.service('PAConnector', ['$websocket', '$rootScope',
    function($websocket, $rootScope) {
        // CONSTANTS
        var self = this;
        
        var websocketUrl = 'wss://localhost:51348';

        
        // Public API.
        this.connected = false;

        this.send = function(message) {
            ws.$$send(message);
        };


        // Initializes the websocket.
        var ws = $websocket.$new({
                url: websocketUrl,
                reconnect: true,
            })
            .$on('$open', function() {
                $rootScope.$apply(function() {
                    self.connected = true;
                });
            })
            .$on('$close', function() {
                $rootScope.$apply(function() {
                    self.connected = false;
                });
            })
            .$on('$message', function(message) {
                $rootScope.$broadcast('PAConnector:onMessage', message);
            });
    }]).run(function(PAConnector) {}); // Ensures that the service is loaded.