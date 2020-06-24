-- wake sequence

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