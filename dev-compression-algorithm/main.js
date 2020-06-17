var bmp = require('bmp-js');
var fs = require('fs');

var bmpBuffer = fs.readFileSync(process.argv[2]);
var bmpData = bmp.decode(bmpBuffer);

var b = [];
var buf = bmpData.data;
var count = 0;
var color = -1;
var over_count = 0;
for (var i = 0; i < buf.length; i += 4) {
    var cur = buf[i + 1];
    if (color != cur || count == 255) {
        if(count == 255) {
            over_count++;
            console.log("alert - over 255", over_count)
        }
        if (color >= 0) {
            b.push(count, color);
        }
        color = cur;
        count = 0;
    }
    count++;
}
if(count > 0) {
    b.push(count, color);
}
console.log(b.length);

fs.writeFileSync(process.argv[2] + ".json", JSON.stringify(b));
fs.writeFileSync(process.argv[2] + ".bin", new Buffer(b))