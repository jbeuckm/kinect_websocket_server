const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 8080 });

// Broadcast to all.
wss.broadcast = function broadcast(data) {
  wss.clients.forEach(function each(client) {
    if (client.readyState === WebSocket.OPEN) {
      client.send(data);
    }
  });
};

/*
wss.on('connection', function connection(ws) {
  ws.on('message', function incoming(data) {
    // Broadcast to everyone else.
    wss.clients.forEach(function each(client) {
      if (client !== ws && client.readyState === WebSocket.OPEN) {
        client.send(data);
      }
    });
  });
});
*/


const spawn = require('child_process').spawn;

var options = {
    cwd: '../Bin/x64-Release'
};


const bat = spawn('./GestureRepeater', [], options);

const rl = require('readline').createInterface({
  input: bat.stdout
});

rl.on('line', (line) => {
  console.log(line);
    wss.broadcast(line);
});

