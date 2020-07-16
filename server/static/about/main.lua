local touch = load_int("about", "page")
if touch > 0 then
    touch = touch - 1
end
sprint("TOUCH UPDATE")
sprint(touch)
smart_draw_r("/about/root.bin", 600, 2400, 0, 800 * touch, 600, 800, 0, 0, 0)