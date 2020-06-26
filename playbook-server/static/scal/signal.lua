

if sig_alert("BEFORE_SLEEP") == 1 and sig_get("RTC_INVALID") == 0 and load_int("cal", "bad") == 1 then
    save_int("cal", "bad", 0)
    loadlib("/scal/main.lua")
end
