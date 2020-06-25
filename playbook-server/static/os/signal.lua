-- wake sequence

local ox = 20
local oy = 20

if sig_alert("WIFI") > 0 or sig_alert("WAKE") > 0 then
    smart_draw("/os/elements-en.bin", 300, 600, 0, 600 - 64, 64, 600, 0 + ox, 0 + oy, 0)
    smart_draw("/os/elements-en.bin", 300, 600, 0, 600 - 300, 64, 600 - 64, 0 + ox, 64 + oy, 0)
    flush_screen(ox, oy, 64, 128, 0)
end

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