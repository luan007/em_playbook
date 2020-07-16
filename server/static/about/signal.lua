if sig_alert("TOUCH_CLICK") > 0 then
    local touch = sig_get("TOUCH_CLICK")
    save_int("about", "page", touch)
    if touch > 0 then
        touch = touch - 1
    end
    sprint("TOUCH UPDATE")
    sprint(touch)
    smart_draw_r("/about/root.bin", 600, 2400, 0, 800 * touch, 600, 800, 0, 0, 0)
end