sprint(
    "JSON Version from LUA " .. json.parse(file_string("/versions")).os.version .. " "
)

sprint(
    "SIG_UX_ENCODER" .. sig_get("ENC_DELTA")
)
sprint(
    "SIG_UX_ENCODER_COUNT" .. sig_get("ENC_COUNT")
)

sig_resolve("ENC_DELTA")
sig_resolve("ENC_COUNT")
sig_resolve("PORTAL_STATE")
sig_resolve("WIFI_STATE")
sig_resolve("WAKE_REASON")
sig_resolve("APP_UPT_STATE")
sig_resolve("BEFORE_SLEEP")

local ox = 20
local oy = 20

if sig_get("APP_SAFE_RENDER") == 2 then
    smart_draw("/os/root.bin", 128, 600, 0, 600 - 64, 64, 600, 0 + ox, 0 + oy, 0)
    smart_draw("/os/root.bin", 128, 600, 0, 600 - 128, 64, 600 - 64, 0 + ox, 64 + oy, 0)
    flush_screen(ox, oy, 64, 128, 1)
else
    sig_set("APP_SAFE_RENDER", 1)
end