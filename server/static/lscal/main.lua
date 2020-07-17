
if sig_get("RTC_INVALID") == 1 then
    smart_draw_r("/lscal/rootERR.bin", 600, 800, 0, 0, 600, 800, 0, 0, 0)
    save_int("cal", "bad", 1)
    return
end

year, month, day, dow, hour, minute, second = now()

sprint(year ..":".. month ..":".. day ..":".. dow )
sprint(hour .. ":" .. minute .. ":" .. second )

-- CALCULATE TOMO
local hour_remain = (23 - hour) 
if hour < 12 then
    hour_remain = 12 - hour
end
local min_remain = (59 - minute)
local sec_remain = (59 - second)
local total_sec = sec_remain + min_remain * 60 + hour_remain * 60 * 60
sprint(total_sec)

smart_draw_r("/lscal/rootBG.bin", 600, 800, 0, 0, 600, 800, 0, 0, 0)

smart_draw_r("/lscal/rootNUM.bin", 180, 4650, 0, (day - 1) * 150, 180, 150, 50, 28, 0)

smart_draw_r("/lscal/rootELM.bin", 120, 1092, 0, (dow) * 36, 120, 36, 240, 50, 0)
smart_draw_r("/lscal/rootELM.bin", 120, 1092, 0, 7 * 36 + 70 * (month - 1), 120, 70, 240, 100, 0)

smart_draw_r("/lscal-news/rootNEWS.bin", 500, 460, 0, 0, 500, 460, 50, 222, 0)

appoint(total_sec)