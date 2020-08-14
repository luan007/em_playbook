var e = require('express')();
const fetch = require('node-fetch');
var qs = require('querystring');
const cheerio = require('cheerio')
const htmlToText = require('html-to-text');
const fs = require("fs");
const YAML = require('json-to-pretty-yaml');

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
	text = text.split("。").map(v=>v.trim().replace(reg, '')).join("。")
	text = text.split("，").map(v=>v.trim().replace(reg, '')).join("，")
	return text.substring(0, 300);
}

var lastUpdate = 0;
var interval = 1000 * 60 * 60 * 6;
var datas = [];
function get_page_0(params) {
	if (Date.now() - lastUpdate < interval) return console.log("skip");
	params.offset = 0;
	params.action = "getmsg";
	//    console.log(params);
	var str = qs.stringify(params);
	   console.log(str);
	fetch('https://mp.weixin.qq.com/mp/profile_ext?' + str).then(res => res.json()).then(async (json) => {
		lastUpdate = Date.now();
		var data = JSON.parse(json.general_msg_list).list;
		//data.length = 3;
		var contents = [];
		for (var i = 0; i < data.length; i++) {
			contents[i] = await fetchUrl(data[i].app_msg_ext_info.content_url);
			console.log(data[i]);
			console.log(contents[i]);
			console.log("---");

			// datas.push({
			// 	...data[i].app_msg_ext_info,
			// 	details: contents[i],
			// 	...data[i].comm_msg_info
			// })

			datas.push({
				content: contents[i],
				link: data[i].app_msg_ext_info.content_url,
				title: data[i].app_msg_ext_info.title,
				desc: getMyDate(data[i].comm_msg_info.datetime).base + "&nbsp;&nbsp;&nbsp;#" + data[i].app_msg_ext_info.author,
			})
		}

		var str = JSON.stringify(datas);
		fs.writeFile("json/google_data.json", str, function (err, data) {
			if (err) {
				console.error(err);
			}
			console.log("----------新增成功-------------");
		});
		var str = YAML.stringify(datas);
		fs.writeFile("yaml/google_data.yaml", str, function (err, data) {
			if (err) {
				console.error(err);
			}
			console.log("----------新增成功-------------");
		});
		//console.log(contents);
	});
}

e.get("*", (req, res) => {
	get_page_0(qs.parse(req.originalUrl.substring(1)));

	fetch('https://mp.weixin.qq.com/mp/profile_ext?action=getmsg' + req.originalUrl.substring(1))
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
		oBase =  oYear + '/' + oMonth + '/' + oDay
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
