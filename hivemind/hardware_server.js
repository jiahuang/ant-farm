var http = require('http'),
    express = require('express'),
    app = express(),
    server = http.createServer(app),
    io = require('socket.io').listen(server);

var browser_socket = null;

app.use('/', express.static(__dirname + '/public'));

app.post('/hardware/:queen_id/:ant_id', function(req, res){
  var timestamp = Date.now();
  var queen_id = decodeURIComponent(req.params.queen_id),
      ant_id = decodeURIComponent(req.params.ant_id);
  console.log("Queen:" + queen_id + " ant:" + ant_id);
  if (browser_socket) {
    browser_socket.emit('visit', {'queen_id':queen_id, 'ant_id':ant_id, 'time':timestamp});
  }
  res.end();
});

server.listen(3000);

// When we get a connection from the browser
io.sockets.on('connection', function (socket) {

  console.log("We are connected!");

  // If there is a socket
  if (socket) {

    // Set our global socket to it for later
    browser_socket = socket;
    // browser_socket.emit('config', config);\
  }

  socket.on('disconnect', function () {
    console.log("We are disconnect.");
    browser_socket = null;
  });
});