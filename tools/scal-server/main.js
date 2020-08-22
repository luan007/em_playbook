const express = require("express");
const app = express();
var bodyParser = require('body-parser');//解析,用req.body获取post参数
var cors = require('cors')
var fs = require('fs');

var cfg = {
    auth: {
        admin: "demo"
    },
    port: 10011,
    script: ""
};

cfg = fs.existsSync(__dirname + "/cfg.json") ? JSON.parse(fs.readFileSync(__dirname + "/cfg.json").toString()) : cfg;
console.log(cfg);

app.use(cors());
app.use('/', express.static('./static'))
app.use(bodyParser.json({ 'limit': '50mb', extended: true }));
app.use(bodyParser.urlencoded({ 'limit': '50mb', extended: true }));

// var filePath = "data/";  
app.post('/upload', function (req, res) {
    console.log(req.body);
    fs.writeFileSync("static/json/data.json", JSON.stringify(req.body));
    res.end();
})

app.post("/form", function (req, res) {
    // console.log(req.body);
    var activity = req.body;
    var old = fs.readFileSync("static/json/history_activity.json", 'utf-8');
    old = JSON.parse(old)
    old.activity.splice(0, 0, activity)
    old.length = 3;
    fs.writeFileSync("static/json/activity.json", JSON.stringify(old.activity[0]));
    fs.writeFileSync("static/json/history_activity.json", JSON.stringify(old));
    res.end();
})


const basicAuth = require('express-basic-auth');

app.use(basicAuth({
    users: cfg.auth,
    challenge: true
}))

app.get('/commit', function (req, res) {
    var result = require('child_process').execFileSync(__dirname + "/" + cfg.script);
    res.end("Done!");
})

var server = app.listen(cfg.port, function () {
    var host = server.address().address;
    var port = server.address().port;

    console.log("应用实例，访问地址为 http://%s:%s", host, port);
})


