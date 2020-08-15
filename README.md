# em_playbook
ESP32 powered oshw, an e-ink thingy with high-def rendering capability &amp; lua sprite engine.


----


To get good rendering result with 800x600 resolution & maintaining rather low-power & (low cost of course), an online + offline hybrid system is designed to keep computation complexity at bare minimum on esp32.

In short, the `app` pipeline is defined as:

0. (Periodic wake) E-Paper thing (esp32) sends request to remote server, requesting latest version of 'app package' if there's any, by comparing stored json.
1. Remote server renders various HTML (Design assets) into BMP (upon request) using `headless chrome`.
2. The BMP is then trimmed & compressed using a modified `RLM` algorithm which allows minimal decoding efforts (JPEG does not perform well here).
3. These images, along with `lua` files, are packed into `tar` & sent to the esp.
4. ESP32 inflates the `tar` -> runs `lua` file if requried.
5. The `lua` file has access to the `sprite engine`.
6. Image is `stitched` quickly & esp goes to sleep.


## /OS

ESP32 Core firmware, with eink driver (slightly modified), with lua engine & power saving logic

TODO: docs

## /server

TODO: docs

## /experiments

TODO: docs

## /hw

TODO: files & docs



## /proxy

TODO: docs

#### 步骤一：按键精灵 [下载](https://dl.pconline.com.cn/html_2/1/59/id=2598&pn=0.html "下载")
* 未完待续

#### 步骤二：Fiddler Everywhere [下载](https://www.telerik.com/download/fiddler/fiddler-everywhere-osx "下载")
* Settings -> HTTPS -> 勾选` Capture HTTPS traffic ` 
* 打开浏览器输入 ` ip地址 + 8888 `，下载证书并双击安装（在电脑端抓包）
* 开启` Auto Responder ` -> Add New Rule -> 
    * MATCH: ` regex:https://mp.weixin.qq.com/mp/profile_ext\?action\=getmsg(\w*) ` 
    * ACTION: ` http://localhost:9999/ `

#### 步骤三：proxy
```
    cd proxy
    node main.js
```
* 指路：[data.json](https://github.com/lulu-s/em_playbook/blob/master/server/static/lscal/news/data.json)