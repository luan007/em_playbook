sprint("renderer")


local render_wifi_id = load_int("os", "wifi", -1)
local prev_render_wifi_id = load_int("os", "wifi_p", -1)
local icon_tray_y = 720

function render_wifi()
    if REASON == "signal" and render_wifi_id==prev_render_wifi_id then
        return
    end

    local wifi_pos_x = 600 - 75 - 60
    local wifi_pos_y = icon_tray_y

    if render_wifi_id == -1 then
        smart_draw_r("/os/elements-en.bin", 600, 300, 6 * 64, 0, 60, 64, wifi_pos_x, wifi_pos_y, 0)
    end
    if render_wifi_id >= 0 then
        smart_draw_r("/os/elements-en.bin", 600, 300, render_wifi_id * 64, 0, 60, 64, wifi_pos_x, wifi_pos_y, 0)
    end

    -- WIFI CONFIGURATOR
    if render_wifi_id == 3 then
        smart_draw_r("/os/elements-en.bin", 600, 300, 0, 128 + 28, 600, 28, 0, 0, 0)
    end
    save_int("os", "wifi_p", render_wifi_id)
end


local bat_pos_x = 600 - 75
local bat_pos_y = icon_tray_y
local render_bat_id = 0
local render_bat_msg = 1 --default: active
local prev_render_bat_msg = load_int("os", "bat_s", -1)

if REASON == "signal" and sig_alert("BEFORE_SLEEP") > 0 then
    render_bat_msg = 0
end

if REASON == "overlay" or render_bat_msg ~= prev_render_bat_msg then
    smart_draw_r("/os/elements-en.bin", 600, 300, render_bat_id * 64, 64, 60, 64, bat_pos_x, bat_pos_y, 0)
    smart_draw_r("/os/elements-en.bin", 600, 300, 4 * 64, 64 + 10 * render_bat_msg, 22, 10, bat_pos_x + 18, bat_pos_y + 42, 0)
    save_int("os", "bat_s", render_bat_msg)
end

render_wifi()