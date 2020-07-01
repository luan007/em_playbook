//test 2, scheme: 1byte encoding
//(000)Color x 8, (00000)

var bmp = require('bmp-js');
var fs = require('fs');

var bmpBuffer = fs.readFileSync(process.argv[2]);
var bmpData = bmp.decode(bmpBuffer);

var b = [];
var buf = bmpData.data;
var count = 0;
var color = -1;
for (var i = 0; i < buf.length; i += 4) {
    var cur = buf[i + 1];
    if (color != cur || count == 32) {
        if (color >= 0) {
            color /= 32;
            b.push(0xFF & (count << 3 | color));
        }
        color = cur;
        count = 0;
    }
    count++;
}
if (count > 0) {
    b.push(0xFF & (count << 3 | color));
}
console.log(b.length);
fs.writeFileSync(process.argv[2] + ".json", JSON.stringify(b));
fs.writeFileSync(process.argv[2] + ".bin", new Buffer(b))