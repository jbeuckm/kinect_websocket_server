const WebSocket = require('ws');

const wss = new WebSocket.Server({
    perMessageDeflate: false,
    port: 8080 
});

// Broadcast to all.
wss.broadcast = function broadcast(data) {
  wss.clients.forEach(function each(client) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(data);
    }
  });
};


const spawn = require('child_process').spawn;

var options = {
    cwd: '../Bin/x64-Release',
    stdio: ["ignore", process.stdout, "ignore"]
};

const bat = spawn('./GestureRepeater', [], options);

const rl = require('readline').createInterface({
  input: process.stdout
});

rl.on('line', function(line) {
    console.log('line');
    console.log(line);
    wss.broadcast(line);
});
