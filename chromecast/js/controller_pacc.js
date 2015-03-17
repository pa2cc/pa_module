'use strict';

var paccControllers = angular.module('paccControllers');

paccControllers.controller('PACCCtrl', ['$scope', '$sce', 'WebRTC',
    function($scope, $sce, WebRTC) {
        // CONSTANTS
        var STATUS_INIT = 'Waiting for a connection...';
        var STATUS_CONNECTED = "";

    
        $scope.audio_url = function() {
            return $sce.trustAsResourceUrl(WebRTC.audio_url);
        };
        
        $scope.status = function() {
            return WebRTC.audio_url == null ? STATUS_INIT : STATUS_CONNECTED;
        };
    }]);