
if sig_get("RTC_INVALID") == 1 then
    smart_draw_r("/scal/rootERR.bin", 600, 800, 0, 0, 600, 800, 0, 0, 0)
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

smart_draw_r("/scal/rootBG.bin", 600, 800, 0, 0, 600, 800, 0, 0, 0)

smart_draw_r("/scal/rootNUM.bin", 325, 8370, 0, (day - 1) * 270, 325, 270, 30, 510, 0)

smart_draw_r("/scal/rootELM.bin", 150, 1064, 0, (dow) * 32, 120, 32, 50, 460, 0)
smart_draw_r("/scal/rootELM.bin", 150, 1064, 0, 7 * 32 + 70 * (month - 1), 150, 70, 230, 440, 0)

news_ver = json.parse(file_string("/scal-news/meta.json"))
if news_ver.showDate.m == month and news_ver.showDate.d == day then
    smart_draw_r("/scal-news/rootNEWS.bin", 500, 185 + 25, 0, 0, 500, 185 + 25, 50, 74, 0)
end

appoint(total_sec)