// silly chrome wants SSL to do screensharing
var fs = require('fs'),
	express = require('express'),
	https = require('https'),
	http = require('http');

var app = express();

app.use(express.static(__dirname));

http.createServer(app).listen(8084);

console.log('running on 0.0.0.0:8084');
