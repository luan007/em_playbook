var exp = require('express');
var tar = require('tar');
var fs = require('fs');
var app = exp();
var comp = require('./bmp-compress');
var puppeteer = require('puppeteer');
const Jimp = require("jimp")
const sharp = require('sharp');
var PORT = 9898;
var APP_ROOT = __dirname + "/static/";
var browser;

function app_prefix(name, file) { return "http://localhost:" + PORT + "/" + name + "/" + file };

async function render_html(url, file_out) {
    console.log("rendering ", url, file_out)
    const page = await browser.newPage();
    await page.goto(url);
    const dimensions = await page.evaluate(() => {
        return {
            width: document.querySelector(".capture") ? document.querySelector(".capture").offsetWidth : document.body.offsetWidth,
            height: document.querySelector(".capture") ? document.querySelector(".capture").offsetHeight : document.body.offsetHeight,
            deviceScaleFactor: window.devicePixelRatio
        };
    });
    console.log('Dimensions:', dimensions);
    var buf = await page.screenshot({
        fullPage: true,
        type: "png",
        encoding: 'binary'
    });
    await page.close();

    buf = await sharp(buf)
        .rotate(90)
        .png()
        .toBuffer()

    console.log("Done Rotation");
    var img = await Jimp.read(buf);
    await img.grayscale();
    await img.writeAsync(file_out);
}

async function preprocess_folder(folder, app) {

    var meta = JSON.parse(fs.readFileSync(folder + "/" + "meta.json").toString());
    var alias = meta.render_alias || {};

    var html_to_render = fs.readdirSync(folder).filter(v => {
        return v.endsWith(".html")
    });

    for (var i = 0; i < html_to_render.length; i++) {
        var cur = html_to_render[i];
        var final = [""];
        if (alias[cur]) {
            final = alias[cur];
        }
        for (var q = 0; q < final.length; q++) {
            var url = app_prefix(app, cur);
            await render_html(url + "#" + final[q], folder + "/" + html_to_render[i].replace(".html", "")
                + final[q] + ".bmp");
        }
    }

    var bmps = fs.readdirSync(folder)
        .filter(v => {
            return v.endsWith(".bmp")
        }).forEach((v) => {
            console.log("Compression");
            comp.compress(folder + '/' + v, folder + '/' + v.replace(".bmp", ".bin"))
            console.log("Compressed..");
        });

    //convert to bin
}

app.get("/app/:name", async (req, res) => {
    var path = APP_ROOT + req.params.name;
    if (!req.params.name || !fs.existsSync(path)) {
        res.status(404).end();
    }
    var app = req.params.name;
    res.set("Connection", "close");
    res.removeHeader("Transfer-Encoding");

    await preprocess_folder(path, app);
    var files = fs.readdirSync(APP_ROOT + req.params.name)
        .filter(v => {
            return v.endsWith(".bin") || v.endsWith(".lua") || v.endsWith(".txt") || v.endsWith(".json")
        });

    tar.c( // or tar.create
        {
            C: path,
            strict: true,
            portable: true,
            gzip: false
        },
        files
    ).pipe(res).on('finish', () => {
        res.end();
    });
});

app.get("/versions", (req, res) => {
    //aggregate meta
    var apps = fs.readdirSync(APP_ROOT);
    var aggr = {};
    for (var i = 0; i < apps.length; i++) {
        var app = apps[i];
        try {
            aggr[app] = JSON.parse(fs.readFileSync(APP_ROOT + app + "/" + "meta.json").toString());
        } catch (e) {

        }
    }
    res.json(aggr);
});

app.use(require('serve-static')("static"));

async function init() {
    console.log("Launching browser")
    browser = await puppeteer.launch();
    render_html('https://baidu.com', "demo.bmp");
    app.listen(PORT);
}


init();