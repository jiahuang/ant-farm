var os = require('os');
var http = require('http');
var sys = require('sys');

var childProcess = require('child_process');

var serialport = require("serialport");

var antfarm_host = 'olinexpo.herokuapp.com';
var hardware_path = '/hardware/'

var SerialPort = serialport.SerialPort;
// var arduino_port = "/dev/tty.usbmodemfd121";
var arduino_port = "/dev/tty.usbmodem1411";
var serialPort = new SerialPort(arduino_port, { 
  parser: serialport.parsers.readline("\n") 
});

// Going to set a trim function for our strings
if(typeof(String.prototype.trim) === "undefined") {
  String.prototype.trim = function () 
  {
    return String(this).replace(/^\s+|\s+$/g, '');
  };
}

serialPort.on("data", function (data) {
  sys.puts("here: "+data);

  // The prefix we set before the uid on the arduino end of things
  var prefix = "Got payload: "; // The prefix before the data we care about comes through

  // If we have a message that matches the prefix
  if (data.indexOf(prefix) == 0) {

    // Grab the message
    message = data.substring(prefix.length).trim();
    console.log("Got a message!", parseInt(message).toString(16));

    var queenId = '1', antId = '2';

    var options = {
      host: antfarm_host,
      path: hardware_path + "?queen_id=" + encodeURIComponent(queenId) + "&ant_id=" + encodeURIComponent(antId)
    };
    http.get(options, function(res) {
      var output = '';
      res.on('error', function(e) {
        console.log('ERROR with hardware post: ' + e.message);
      });

      console.log("hardware post status:" + res.statusCode);
      // if we were able to check in (fob ID recognized already)
      if (res.statusCode == 200) {
        console.log("Okay, the response means everything is good.");

      } 
    }); // end of http.get
  }
});