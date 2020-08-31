-- wake sequence

if _G["led_c"] ~= nil then
    _G["led_c"](1, 1)
end

if sig_alert("WAKE") > 0 then
    save_int("ux", "tick", 0)
end

if sig_alert("NOTIFY_RELEASE") > 0 and sig_get("NOTIFY_RELEASE") > 0 then
    smart_draw_r("/os/elements-en.bin", 600, 300, 0, 128, 600, 28, 0, 0, 0)
end

if sig_alert("APP_UPT_STATE") > 0 and sig_get("APP_UPT_STATE") == 1 then
    smart_draw_r("/os/elements-en.bin", 600, 300, 0, 128 + 28 * 5, 600, 28, 0, 0, 0)
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

    if wifi_id == 2 and sig_get("RTC_INVALID") == 1 then 
        --total fail
        smart_draw_r("/shared/wizardWIFI.bin", 600, 800, 0, 0, 600, 800, 0, 0, 0)
    end
end

if wifi_id ~= -1 then
    save_int("os", "wifi", wifi_id)
end

function mod(a, b)
    return a - (math.floor(a/b)*b)
end

local ux_m = load_int("ux", "menu")
if sig_alert("SW_UP") > 0 and ux_m > 0 then
    req_redraw(1)
    sig_clear("SW_UP")
end

local show_menu_force = 0
if sig_alert("SW_UP") > 0 and ux_m == 0 then
    show_menu_force = 1
    sig_clear("SW_UP")
end

if sig_alert("ENC_COUNT") > 0 or show_menu_force == 1 then
    local debounce = load_int("ux", "tick")
    local prev_count = load_int("ux", "pc")
    if show_menu_force == 1 or debounce <= 0 or ((millis() / 100) - debounce) > 12 then
        local selnum = load_int("ux", "sel")
        local d = sig_get("ENC_COUNT")
        if (d - prev_count) > 0 then
            selnum = selnum - 1
        elseif (d - prev_count) < 0 then
            selnum = selnum + 1
        end
        save_int("ux", "pc", d)
        
        local vers = json.parse(file_string("/versions"))

        local apps = {}
        for key, value in pairs(vers) do --pseudocode
            if value.capability["asset-pack"] ~= 1 and key ~= "os" then
                apps[#apps+1]=key
            end
        end

        table.sort( apps )

        local len = #apps
        if selnum < 0 then
            -- selnum = len - 1
            selnum = 0
        elseif selnum >= len then
            -- selnum = 0
            selnum = len - 1
        end
        save_int("ux", "sel", selnum)
        local picked = selnum

        sprint("MENU SELECT ******* " .. apps[picked + 1] .. selnum)

        if(load_string("main", "APP") ~= apps[picked + 1]) or ux_m == 0 then
            save_string("main", "APP", apps[picked + 1])
            save_int("ux", "menu", 1)
            led_c(0, 1)
            led_c(1, 1)
            for i=1,len do
                local v = i * 2 - 2
                if i == (picked + 1) then
                    v = v + 1
                end
                smart_draw_r("/os-apps/tray-en.bin", 125, 26 * len * 2, 0, 26 * v, 125, 26, 600 - 125, 26 * (i - 1) + 70, 0)
            end
            save_int("ux", "tick", math.floor(millis() / 100))
        end
        req_redraw(millis() + 2000)
    end
    sig_clear("ENC_COUNT")
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
-- sprint("Clearing all signal")



if _G["led_c"] ~= nil then
    _G["led_c"](1, 0)
end