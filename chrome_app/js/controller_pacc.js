'use strict';

var paccControllers = angular.module('paccControllers');

paccControllers.controller('PACCCtrl', ['$scope', 'CCConnector', 'PAConnector',
    function($scope, CCConnector, PAConnector) {
        $scope.CCConnector = CCConnector;
        $scope.PAConnector = PAConnector;
    }]);