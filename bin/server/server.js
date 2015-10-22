// server needs to run before application
var express = require('express'),
	app = express(),
	http = require('http').Server(app),
	io = require('socket.io')(http),
	dgram = require('dgram'),
	osc = require('osc-min'),
	os = require('os'),
	ifaces = os.networkInterfaces(),
	// osc is a thin layer on top of udp
	sendSocket = dgram.createSocket({type:"udp4",reuseAddr:true}),
	receiveSocket = dgram.createSocket({type:"udp4",reuseAddr:true}),
	SEND_PORT = 12345,
	RECIEVE_PORT = 12346;

sendSocket.bind(SEND_PORT);
recieveSocket.bind(RECIEVE_PORT);

function getOSCMessage(msg){
	var oscMessage = osc.fromBuffer(msg);

	try{
		//translate osc buffer into javascript object
		var element = oscMessage.elements[0],
		address = element.address,
		args = element.args; //contains 'type' and 'value

		return {
			address: address,
			filename: args[0].value,
			id : args[1].value
		}

	}catch(error){
		console.log("invalid OSC packet");
	}
}

function sendOSCMessage(photoType, socketId){
	var buffer = osc.toBuffer({
		address: '/take/picture',
		args : [{
			type: "string",
			value: photoType
		},
		{
			type: "string",
			value: socketId
		}]
	});

	sendSocket.send(buffer, 0, buffer.length, SEND_PORT, 'localhost');
}

recieveSocket.on('message', function(message, remote){
	var messageValues = getOSCMessage(message);
	switch(messageValues.address){
		case '/transmit/photo':
			//twitter upload happens here. At the end of the process, send the message and file
			// back to the main application
			io.sockets.to(messageValues.id).emit('transmit photo', messageValues.filename);
			break;
		case '/photo/countdown':
			io.sockets.to(messageValues.id).emit('countdown timer', messageValues.filename);
			break;
		default:
			console.log(messageValues);
	}
});

// express setup
app.use("/js", express.static(__dirname + '/js'));
app.use("/css", express.static(__dirname + '/css'));
//this should be the data folder for the oF app
app.use("/img", express.static(__dirname + '/../data/output'));



//websockets
io.on('connection', function(socket){
	console.log('a user connected');

	socket.on('take photo', function(message){
		console.log('take photo', message, socket.id);
		sendOSCMessage(message.type, socket.id);
	});

	socket.on('disconnect', function(){
		console.log('a user disconnected');
	});
});

app.get('/', function(req, res){
	res.sendfile(__dirname + '/views/index.html');
});

http.listen(3000, function() {
	console.log('Listening on port', http.address().port);

	for (var dev in ifaces) {
		var alias=0;
		ifaces[dev].forEach(function(details){
			if (details.family=='IPv4') {
				console.log(dev+(alias?':'+alias:''),details.address);
				++alias;
			}
		});
	}
});