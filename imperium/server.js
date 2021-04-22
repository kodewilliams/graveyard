var compression = require('compression')
var express = require('express')
var app = express()
var quotes = require('./quotes.json')
var rn = require('random-number')
var path = require('path')
var gen = rn.generator({min: 0, max: 5420, integer: true})
var cors = require('cors')

app.use(cors())
app.use(compression())
app.use(express.static(path.join(__dirname, 'public')));


app.get('/', function (req, res) {
  res.send('Welcome to Motiv8')
})

app.get('/quotes', function(req, res, next) {
	var randomNumbers = [];
	for (var i=0; i<8; i++)
		randomNumbers.push(gen())
	var returnedQuotes = [];
	for (var i=0; i<8; i++)
		returnedQuotes.push(quotes[randomNumbers[i]])

	res.send(returnedQuotes)
})

app.listen(3000);
console.log('App running at port 3000.');
