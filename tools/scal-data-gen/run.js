var fs =require("fs");
var text = fs.readFileSync('script.txt').toString()
var robot = require("robotjs");


var str = text.split("\n");

var data = [], tem = null, args = null;
for(let i = 0; i < str.length; i++){
    tem = str[i].trim().split(" ");
    args = JSON.parse(JSON.stringify(tem));
    args.shift();
    args = args.map( a => a.replace(/,/g, '') * 1 );
    data.push(
        {type: tem[0], args}
    )
}

console.log(data);


runCommand(data, 0);


function runCommand(data, i) {
    var obj = data[i];
    if(!obj) return;
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
            
        case 'PageDown':
            robot.keyTap('pagedown');
            console.log('pd');
            break;
        case 'MouseWheel':
            console.log(obj.args[0]);
            robot.scrollMouse(0, obj.args[0]);
            // console.log('MouseWheel');
            break;
        case 'Delay':
            // console.log('Delay');
            // robot.setKeyboardDelay(obj.args[0] * 1000)
            return setTimeout(()=>{
                console.log("len :", data.length, i + 1);
                runCommand(data, i+1);
            }, obj.args[0])
            break;
        default:
            console.log("未知命令: ", obj.type);
    }
    console.log(i);
    return runCommand(data, i+1);
}