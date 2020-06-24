

-- wake sequence

if sig_raised("WAKE_REASON") > 0 then
    sprint(
        "WAKE UP, CLEAR INTERNAL COUNTER"
    )
end


if sig_raised("BEFORE_SLEEP") > 0 then
    local taint = load_int("main", "TAINT")
    sprint(
        "TAINTED = " .. taint
    )
end


sprint(
    sig_raised("ENC_COUNT")
)

local ox = 20
local oy = 20

if sig_raised("ENC_COUNT") == 1 then
    if sig_get("APP_SAFE_RENDER") == 2 then
        save_int("main", "TAINT", 1)
        smart_draw("/os/root.bin", 128, 600, 0, 600 - 64, 64, 600, 0 + ox, 0 + oy, 0)
        smart_draw("/os/root.bin", 128, 600, 0, 600 - 128, 64, 600 - 64, 0 + ox, 64 + oy, 0)
        flush_screen(ox, oy, 64, 128, 1)
        sig_set("APP_REFRESH_REQUEST", millis() + 1500) 
        -- refresh after 1.5s
    else
        sig_set("APP_SAFE_RENDER", 1)
    end
end

sig_resolve("ENC_DELTA")
sig_resolve("ENC_COUNT")
sig_resolve("PORTAL_STATE")
sig_resolve("WIFI_STATE")
sig_resolve("WAKE_REASON")
sig_resolve("APP_UPT_STATE")
sig_resolve("BEFORE_SLEEP")
