var fs =require("fs");
var text = fs.readFileSync('script.txt');
var robot = require("robotjs");


var str = text.toString().split("\r\n");

var data = [], tem = null, args = null;
for(let i = 0; i < str.length; i++){
    tem = str[i].split(" ");
    args = JSON.parse(JSON.stringify(tem));
    args.shift();
    args = args.map( a => a.replace(/,/g, '') * 1 );
    data.push(
        {type: tem[0], args}
    )
}

// var delay = 2000;
for(let i = 0; i < data.length; i++){
    // setTimeout(()=>{
        runCommand(data[i])
    // }, delay) 
}

function runCommand(obj) {
    switch(obj.type){
        case 'MoveTo':
            robot.moveMouse(obj.args[0], obj.args[1]);
            // console.log('MoveTo');
            break;
        case 'LeftDown':
        case 'LeftClick':
            robot.mouseClick();
            // console.log('LeftClick');
            break;
        case 'MouseWheel':
            robot.scrollMouse(0, obj.args[0]);
            // console.log('MouseWheel');
            break;
        case 'Delay':
            // console.log('Delay');
            // robot.setKeyboardDelay(obj.args[0] * 1000)
            robot.setMouseDelay(obj.args[0])
            break;
        default:
            console.log("未知命令: ", obj.type);
    }
}