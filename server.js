const serial = require('serialport');
if(process.argv.length < 3) {
	console.log('Usage: node server.js <path> [port]');
	serial.SerialPort.list().then(ports=>{
		console.log(ports.filter(p=>{return (p.manufacturer || p.productId);}));
		process.exit();
	}).catch(e=>{
		console.error(e);
		process.exit();
	});
	return;
}

const serialPath = process.argv[2];
const PORT = parseInt(/^\d+$/.test(process.argv[3]) ? process.argv[3] : '2023');
const fs = require('fs');
const net = require('net');
const iconv = require('iconv-lite');
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
	path: serialPath,
	baudRate: 230400,
	dataBits: 8,
	parity: 'even',
	stopBits: 1,
	rtscts: false,
	autoOpen: false
});
const setRTC = function() {
	const d = new Date();
	serialPort.write('rtc ' + (d.getYear() + 1900) + ' ' + (d.getMonth() + 1) + ' ' + d.getDate() + ' ' + d.getHours() + ' ' + d.getMinutes() + ' ' + d.getSeconds() + '\r\n', function(e) {
		if(e) console.error(e);
	});

	setTimeout(setAlarm, 100);
};
const setAlarm = function() {
	const d = new Date(new Date().getTime() + 50000);
	serialPort.write('alarm ' + (d.getYear() + 1900) + ' ' + (d.getMonth() + 1) + ' ' + d.getDate() + ' ' + d.getHours() + ' ' + d.getMinutes() + ' ' + d.getSeconds() + '\r\n', function(e) {
		if(e) {
			console.error(e);
			process.exit();
		} else {
			setTimeout(setAlarm, 60000); // 设置一分钟后的闹钟
		}
	});
}

serialPort.open(function(e) {
	if(e) {
		console.log('open serial '  + serialPath + ' failure', e);
		process.exit();
	} else {
		console.log('open serial '  + serialPath + ' success');
	}
	
	setTimeout(setRTC, 100);
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

server.listen(PORT, function() {
	console.log('Serial Service listen port is ' + PORT);
});

//捕获异常，防止崩溃
process.on('uncaughtException', function (e) {
	console.error(e);
});
