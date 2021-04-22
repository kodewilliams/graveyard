var app = angular.module('motiv8', []);

app.controller('QuotesController', ['$http', '$scope', function($http, $scope) {
	var quotesUrl = window.location.href + "quotes";
	var getQuotes = $http.get(quotesUrl)
		.then(function(response) {
			return response.data;
		}, function(err) {
			throw err;
		}
	);

	getQuotes.then(function(quotes) {
		$scope.quotes = quotes;
	});
	
	console.log('Motiv8 front end running.');
}]);
