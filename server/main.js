var exp = require('express');
var tar = require('tar');
var fs = require('fs');
var app = exp();
var comp = require('./bmp-compress');
var puppeteer = require('puppeteer');
const Jimp = require("jimp")
const sharp = require('sharp');
var PORT = process.argv[2] || 9898;
var TIMEOUT = process.argv[3] || (60 * 30);
var APP_ROOT = __dirname + "/static/";
var browser;

var next_reboot = Date.now() + 1000 * TIMEOUT;
var lock = 0;

function lock_reboot() {
    lock++;
}
function safe_to_reboot() {
    lock--;
}

setInterval(() => {
    //ensures reboot
    if (lock <= 0 && Date.now() > next_reboot) {
        browser && browser.close();
        process.exit();
    }
}, 1000);

function app_prefix(name, file) { return "http://localhost:" + PORT + "/" + name + "/" + file };

function file_cached(file, cache) {
    if (fs.existsSync(file) && Date.now() - fs.statSync(file).mtime.getTime() < (cache * 1000)) {
        return true;
    }
    return false;
}

async function render_html(url, file_out, cache) {
    if (file_cached(file_out, cache)) {
        console.log("render cached", url);
        return;
    }
    console.log("rendering ", url, file_out);
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
        .extract({ left: 0, top: 0, width: dimensions.width, height: dimensions.height })
        .rotate(-90)
        .png()
        .toBuffer()

    console.log("Done Rotation");
    var img = await Jimp.read(buf);
    await img.grayscale();
    await img.writeAsync(file_out);
}

async function preprocess_folder(folder, app, work_folder) {
    work_folder = work_folder || "";
    var sub_folder = (work_folder ? (work_folder + "/") : "");
    var meta = JSON.parse(fs.readFileSync(folder + "/" + sub_folder + "meta.json").toString());
    var alias = meta.render_alias || {};
    var cache = meta.cache || 0;

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
            await render_html(url + "#" + final[q], folder + "/" + sub_folder + html_to_render[i].replace(".html", "")
                + final[q] + ".bmp", cache || 0);
        }
    }
    var bmps = fs.readdirSync(folder + '/' + sub_folder)
        .filter(v => {
            return v.endsWith(".bmp")
        }).forEach((v) => {
            if (file_cached(folder + '/' + sub_folder + v.replace(".bmp", ".bin"), cache)) {
                return;
            }
            console.log("Compression");
            comp.compress(folder + '/' + sub_folder + v, folder + '/' + sub_folder + v.replace(".bmp", ".bin"))
            console.log("Compressed..");
        });

    //convert to bin
}

app.get("/app/:name", async (req, res) => {
    lock_reboot();
    var app = req.params.name.split("-")[0];
    var sub = req.params.name.split("-")[1] || "";
    var path = APP_ROOT + app;
    var real_path = APP_ROOT + app + (sub ? "/" : "") + sub;
    console.log(path);
    if (!req.params.name || !fs.existsSync(path)) {
        res.status(404).end();
    }
    res.set("Connection", "close");
    res.removeHeader("Transfer-Encoding");
    console.log(path);
    await preprocess_folder(path, app, sub);
    var files = fs.readdirSync(real_path)
        .filter(v => {
            return v.endsWith(".bin") || v.endsWith(".lua") || v.endsWith(".txt") || v.endsWith(".json")
        });

    tar.c( // or tar.create
        {
            C: real_path,
            strict: true,
            portable: true,
            gzip: false
        },
        files
    ).pipe(res).on('finish', () => {
        safe_to_reboot();
        res.end();
    });
});

app.get("/versions", (req, res) => {
    //aggregate meta
    lock_reboot();
    var apps = fs.readdirSync(APP_ROOT);
    var aggr = {};
    for (var i = 0; i < apps.length; i++) {
        var app = apps[i];
        if (!fs.existsSync(APP_ROOT + app + "/" + "meta.json")) continue;
        try {
            aggr[app] = JSON.parse(fs.readFileSync(APP_ROOT + app + "/" + "meta.json").toString());
            var sub_packs = fs.readdirSync((APP_ROOT + app));
            sub_packs.forEach(v => {
                if (fs.existsSync(APP_ROOT + app + "/" + v + "/meta.json")) {
                    aggr[app + "-" + v] = JSON.parse(fs.readFileSync(APP_ROOT + app + "/" + v + "/meta.json").toString());
                }
            });
        } catch (e) {
        }
    }
    safe_to_reboot();
    res.json(aggr);
});

app.use(require('serve-static')("static"));

async function init() {
    console.log("Launching browser")
    browser = await puppeteer.launch({
        args: ['--no-sandbox']
    });
    render_html('https://baidu.com', "demo.bmp");
    app.listen(PORT);
}


init();