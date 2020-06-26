-- wake sequence

if sig_alert("NOTIFY_RELEASE") > 0 and sig_get("NOTIFY_RELEASE") > 0 then
    smart_draw_r("/os/elements-en.bin", 600, 300, 0, 128, 600, 28, 0, 0, 0)
end

local wifi_state = sig_get("WIFI")
-- local wifi_ui_state = load_int("ux", "wifi", -1)
wifi_id = -1

-- smart_draw("/os/elements-en.bin", 300, 600, 128, 0, 128 + 27, 300, 0, 0, 0)
if sig_alert("WIFI") > 0 and wifi_state ~= WIFI_IDLE then
    if wifi_state == WIFI_CONFIG then
        wifi_id = 3
    elseif wifi_state == WIFI_CONNECTING  then
        wifi_id = 4
    elseif wifi_state == WIFI_FAILED  then
        save_int("ux", "wlan_e", -1)
        wifi_id = 5
    elseif wifi_state == WIFI_SUCC then
        save_int("ux", "wlan_e", 1)
        wifi_id = 1
    end
end

if sig_alert("BEFORE_SLEEP") > 0 then
    if wifi_state == WIFI_IDLE or wifi_state == WIFI_SUCC then
        wifi_id = 0
    elseif wifi_state == WIFI_IDLE and load_int("wlan_e") > 0 then --wifi not activated, preserve last info
        wifi_id = 0
    elseif wifi_state == WIFI_IDLE and load_int("wlan_e") < 0 then --wifi not activated, preserve last info
        if sig_get("WIFI_TRY") > 2 then
            wifi_id = 2
        else
            wifi_id = 5
        end
    elseif wifi_state == WIFI_FAILED and sig_get("WIFI_TRY") > 2 then
        wifi_id = 2
    end
end

if wifi_id ~= -1 then
    save_int("os", "wifi", wifi_id)
end

loadlib("/os/renderer.lua")

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


