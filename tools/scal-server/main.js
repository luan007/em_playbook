const express = require("express");
const app = express();
var bodyParser = require('body-parser');//解析,用req.body获取post参数
var cors = require('cors')
var fs = require('fs');

app.use(cors());
app.use('/', express.static('./static'))
app.use(bodyParser.json({'limit': '50mb', extended: true}));
app.use(bodyParser.urlencoded({'limit': '50mb', extended: true}));

// var filePath = "data/";  
app.post('/upload', function(req, res) {
    console.log(req.body);
    fs.writeFileSync("static/json/data.json", JSON.stringify(req.body));
    res.end();
})


app.get('/commit', function(req, res) {
    var old = fs.readFileSync("static/json/data.json");
    fs.writeFileSync("static/json/newdata.json", old)
    res.end();
})

var server = app.listen(8081, function() {
    var host = server.address().address;
    var port = server.address().port;
   
    console.log("应用实例，访问地址为 http://%s:%s", host, port);
})   


