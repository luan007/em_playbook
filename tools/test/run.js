const { exec } = require('child_process');
const fs = require("fs");
var text = fs.readFileSync('script.txt').toString()
// var robot = require("robotjs");

var str = text.split("\n");
var data = [], tem = null, args = null;
for (let i = 0; i < str.length; i++) {
	tem = str[i].trim().split(" ");
	args = JSON.parse(JSON.stringify(tem));
	args.shift();
	args = args.map(a => a.replace(/,/g, '') * 1);
	data.push(
		{ type: tem[0], args }
	)
}

function runCommand(data, i, cb) {
	busy = true;
	var obj = data[i];
	var x = 0, y = 0;
	if (data.length <= i) {
		busy = false;
		return cb();
	}
	if (!obj) return;
	switch (obj.type) {
		case 'MoveTo':
			exec(`nircmd sendmouse move ${obj.args[0]} ${obj.args[1]}`, function(error, stdout, stderr) {
				if(error){
					console.error(error);
				}
				else{
					console.log("send MoveTo", obj.args[0], obj.args[1]);
				}
			});
		
			// robot.moveMouse(obj.args[0], obj.args[1] );
			// console.log("send MoveTo");
			// console.log("send MoveTo: " + obj.args[0] + "," + obj.args[1]);
			break;
		case 'LeftDown':
		case 'LeftClick':
			// robot.mouseClick();
			exec("nircmd sendmouse left click", function(error, stdout, stderr) {
				if(error){
					console.error(error);
				}
				else{
					console.log("send LeftClick");
				}
			});
			break;
		case 'PageDown':
			// exec("nircmd sendmouse left click", function(error, stdout, stderr) {
			// 	if(error){
			// 		console.error(error);
			// 	}
			// 	else{
			// 		console.log("send LeftClick");
			// 	}
			// });
			// robot.keyTap('pagedown');
			console.log("send pagedown");
			break;
		case 'MouseWheel':
			// robot.scrollMouse(0, obj.args[0]);
			console.log("send MouseWheel");
			break;
		case 'Delay':
			return setTimeout(() => {
				console.log("send Delay");
				runCommand(data, i + 1, cb);
			}, (obj.args[0] + Math.floor(Math.random() * 100)) )
			break;
		case 'Random': 
			x = Math.floor(Math.random() * 1000);
			y = Math.floor(Math.random() * 650);
			// robot.moveMouse(x, y);
			exec(`nircmd sendmouse move ${x} ${y}`, function(error, stdout, stderr) {
				if(error){
					console.error(error);
				}
				else{
					console.log("send Random: " + x + "," + y);
				}
			});
			break;
		default:
			console.log("unknown: ", obj.type);
	}
	return runCommand(data, i + 1, cb);
}


runCommand(data, 0, ()=>{
    console.log("ok");
})