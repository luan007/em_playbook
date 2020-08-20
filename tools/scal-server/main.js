const express = require("express");
const app = express();
var bodyParser = require('body-parser');//解析,用req.body获取post参数
app.use(cors());
var fs = require('fs');


var filePath = "data/";  
app.post('/download', function(req, res) {

})


var server = app.listen(8081, function() {
    var host = server.address().address;
    var port = server.address().port;
   
    console.log("应用实例，访问地址为 http://%s:%s", host, port);
})   

