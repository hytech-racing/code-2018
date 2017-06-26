var express = require('express');
var app = express();
var http = require('http').Server(app);
var io = require('socket.io')(http);

var can = require('socketcan');
var channel = can.createRawChannel("can0", true);

app.use(express.static(__dirname + '/public'));

var watchers = io.of('/');

watchers.on('connection', function(socket){
    console.log('Watcher connected');
    socket.on('disconnect', function() {
        console.log('Watcher disconnected');
    });
});

channel.addListener("onMessage", function(msg) {
    //console.log(msg);
    watchers.emit('canMessage', msg);
});
channel.start();

http.listen(3000, function(){
  console.log('listening on port 3000');
});
