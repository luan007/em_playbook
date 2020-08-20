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


