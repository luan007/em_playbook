var e = require('express')();
const fetch = require('node-fetch');
var qs = require('querystring');
const cheerio = require('cheerio')
const htmlToText = require('html-to-text');
const fs = require("fs");
const YAML = require('json-to-pretty-yaml');
var argv = require('yargs').argv;
const { timeEnd } = require('console');

var fs = require("fs");
var text = fs.readFileSync('script.txt').toString()
var robot = require("robotjs");

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

var busy = false;

function runCommand(data, i, cb) {
	busy = true;
	var obj = data[i];
	if (data.length <= i) {
		busy = false;
		return cb();
	}
	if (!obj) return;
	switch (obj.type) {
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
			return setTimeout(() => {
				console.log("len :", data.length, i + 1);
				runCommand(data, i + 1, cb);
			}, obj.args[0])
			break;
		default:
			console.log("未知命令: ", obj.type);
	}
	return runCommand(data, i + 1, cb);
}

async function mouseControls() {
	return new Promise((res, rej) => {
		runCommand(data, 0, res)
	});
}


// console.log(argv);

async function fetchUrl(url) {
	var a = await fetch(url);
	var txt = await a.text();
	const $ = cheerio.load(txt);
	var text = "";
	text = htmlToText.fromString($(".rich_media_content").html(), {
		ignoreHref: true,
		ignoreImage: true,
		noLinkBrackets: true
	});
	var match = /^(http|\*|文\s|作者\s|转载\s|原创\s)/i;
	text = text.split("\n").filter(
		(t) => {
			if (match.test(t.trim())) return false;
			if (t.trim() == "") return false;
			return true;
		}
	).map(v => v.trim());
	// console.log(text.join())
	text = text.join().replace(/,/g, ' ');
	var reg = new RegExp(/^([`~!@#$^&*()=|{}':;',\[\].<>/?~！@#￥……&*（）——|{}【】‘；：”“'。，、？])/, 'i');
	text = text.split("。").map(v => v.trim().replace(reg, '')).join("。")
	text = text.split("，").map(v => v.trim().replace(reg, '')).join("，")
	return text.substring(0, 300);
}


// 
var lastUpdate = 0;
var interval = 1000 * 60 * 60 * 6;
function skip_update() {
	return Date.now() - lastUpdate < interval;
}
async function get_page_0(params) {
	if (skip_update()) return console.log("skip");
	try {
		var datas = { items: [] };
		lastUpdate = Date.now();
		params.offset = 0;
		params.action = "getmsg";
		var str = qs.stringify(params);
		// console.log(str)
		var r = await fetch('https://mp.weixin.qq.com/mp/profile_ext?' + str);
		var json = await r.json();

		var data = JSON.parse(json.general_msg_list).list;
		data.length = 3;
		var contents = [];
		var url = null;
		for (var i = 0; i < data.length; i++) {
			contents[i] = await fetchUrl(data[i].app_msg_ext_info.content_url);
			// console.log(data[i]);
			// console.log(contents[i]);
			console.log("---");
			url = data[i].app_msg_ext_info.content_url.replace(/amp;/g, "").split("?")
			url[1] = qs.parse(url[1]);
			url[1] = {
				__biz: url[1].__biz,
				mid: url[1].mid,
				sn: url[1].sn,
				idx: url[1].idx
			}
			url[1] = qs.stringify(url[1]);
			datas.items.push({
				content: contents[i],
				link: url.join("?"),
				title: data[i].app_msg_ext_info.title,
				desc: getMyDate(data[i].comm_msg_info.datetime * 1000).base + "&nbsp;&nbsp;&nbsp;#" + data[i].app_msg_ext_info.author,
			})
		}
		// if(datas.items != null) {
		var str = JSON.stringify(datas);
		fs.writeFileSync("json/data.json", str);
		// } else {
		// 	lastUpdate = 0;
		// 	clickHistoryList();
		// 	get_page_0(params);
		// }

		// var str = YAML.stringify(datas);
		// fs.writeFile("yaml/google_data.yaml", str, function (err, data) {
		// 	if (err) {
		// 		console.error(err);
		// 	}
		// 	console.log("----------新增成功-------------");
		// });
		//console.log(contents);
	} catch (e) {
		lastUpdate = 0;
		console.error(e);
	}
}


e.get("*", (req, res) => {
	get_page_0(qs.parse(req.originalUrl.substring(1)));
	return fetch('https://mp.weixin.qq.com/mp/profile_ext?action=getmsg' + req.originalUrl.substring(1))
		.then(res => res.json())
		.then(json => res.json(json).end());
});

e.listen(9999);

// 转换日期
function getMyDate(str) {
	var oDate = new Date(str),
		oYear = oDate.getFullYear(),
		oMonth = oDate.getMonth() + 1,
		oDay = oDate.getDate(),
		oHour = oDate.getHours(),
		oMin = oDate.getMinutes(),
		oSen = oDate.getSeconds(),
		oBase = oYear + '/' + oMonth + '/' + oDay
	return {
		years: oYear,
		months: oMonth,
		days: oDay,
		hours: oHour,
		minutes: oMin,
		seconds: oSen,
		base: oBase,
		ms: oDate.getTime()
	}
}

setInterval(() => {
	if (busy) return;
	if (skip_update()) return;
	mouseControls();
}, 30000);