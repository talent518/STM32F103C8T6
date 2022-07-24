const serial = require("serialport");
const fs = require('fs');
const net = require('net');
const iconv = require('iconv-lite');
const config = require('./config');
const conns = [];
const server = net.createServer(function(conn) {
	conns.push(conn);
	conn.on('data', function(data) {
		data = Buffer.from(data);
		serialPort.write(data, function(e) {
			if(e) console.error(e);
		});
	});
	conn.on('close', function() {
		conns.splice(conns.indexOf(conn), 1);
	});
});
const serialPort = new serial.SerialPort({
	path: config.com,
	baudRate: config.baudRate,
	autoOpen: false
});

serialPort.open(function(e) {
	if(e) {
		console.log('open serial '  + config.com + ' failure', e);
		process.exit();
	} else {
		console.log('open serial '  + config.com + ' success');
	}
	const d = new Date();
	serialPort.write('rtc ' + (d.getYear() + 1900) + ' ' + (d.getMonth() + 1) + ' ' + d.getDate() + ' ' + d.getHours() + ' ' + d.getMinutes() + ' ' + d.getSeconds() + '\r\n', function(e) {
		if(e) console.error(e);
	});
	setInterval(function() {
		const d2 = new Date();
		serialPort.write('alarm ' + (d2.getYear() + 1900) + ' ' + (d2.getMonth() + 1) + ' ' + d2.getDate() + ' ' + d2.getHours() + ' ' + d2.getMinutes() + ' ' + d2.getSeconds() + '\r\n', function(e) {
			if(e) console.error(e);
		});
	}, 60000); // ÿ��������һ������
});

serialPort.on('data', function(data) {
	data = Buffer.from(data);
	const str = iconv.decode(data, 'gbk');
	process.stdout.write(str);
	conns.forEach(function(conn, i) {
		conn.write(str);
	});
});

process.stdin.on('data', function(data) {
	data = Buffer.from(data);
	serialPort.write(data, function(e) {
		if(e) console.error(e);
	});
});

server.listen(config.port, function() {
	console.log('Serial Service listen port is ' + config.port);
});

//�����쳣����ֹ����
process.on('uncaughtException', function (e) {
	console.error(e);
});
