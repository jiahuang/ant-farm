var browser_socket = io.connect('http://localhost:3000');
var first_time = null, last_time = null;
var oldata =null;
browser_socket.on('visit', function (data) {

  // console.log("Received a visit!");
  // console.log(data);
  queenId = data.queen_id;
  antId = data.ant_id;
  time = data.time;
  olddata = data;
  if (time < first_time || time > last_time + 2000) {
    first_time = time;
    console.log(first_time);
  }
  if (time > last_time) {
    last_time = time;
  }
  var timespent = (last_time - first_time)/1000;
  // $.post("http://thepaulbooth.com:3727/newuid/" + data.uid);
  $('#displaytext').text("Ant " + antId + " has been at queen " + queenId + " for " + timespent + " seconds.");
});