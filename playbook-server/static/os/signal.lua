-- wake sequence

local ox = 700
local oy = 20

if sig_alert("NOTIFY_RELEASE") > 0 and sig_get("NOTIFY_RELEASE") > 0 then
    smart_draw_r("/os/elements-en.bin", 600, 300, 0, 128, 600, 28, 0, 0, 0)
end

-- smart_draw("/os/elements-en.bin", 300, 600, 128, 0, 128 + 27, 300, 0, 0, 0)
if sig_alert("WIFI") > 0 then
    local wifi_state = sig_get("WIFI")
    local wifi_pos_x = 600 - 100
    local wifi_pos_y = 700
    local wifi_id = 0
    if wifi_state == WIFI_CONFIG then
        wifi_id = 3
        smart_draw_r("/os/elements-en.bin", 600, 300, 0, 128 + 28, 600, 28, 0, 0, 0)
    elseif wifi_state == WIFI_CONNECTING  then
        wifi_id = 4
    elseif wifi_state == WIFI_FAILED  then
        wifi_id = 2
    elseif wifi_state == WIFI_SUCC then
        wifi_id = 0
    elseif wifi_state == WIFI_IDLE then
        wifi_id = 1
    end
    smart_draw_r("/os/elements-en.bin", 600, 300, wifi_id * 64, 0, 60, 64, wifi_pos_x, wifi_pos_y, 0)
end

sig_clear("ENC_COUNT")
sig_clear("SYS_MSG")
sig_clear("WAKE")
sig_clear("BEFORE_SLEEP")
sig_clear("RTC_INVALID")
sig_clear("WIFI")
sig_clear("NOTIFY_RELEASE")
sig_clear("NO_MORE_OP")
sig_clear("APP_UPT_STATE")
sig_clear("SYS_BROKE", 0)

sprint("Clearing all signal")