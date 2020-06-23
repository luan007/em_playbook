#ifndef _GUARD_H_SYS
#define _GUARD_H_SYS

#include "hal-fs.h"
#include "hal-display.h"
#include "hal-io.h"
#include "hal-nap.h"
#include "shared.h"

////////////ALL SIGNALS
String message;
void fallback_renderer()
{
    bool changed = false;
    bool sys_broke = false;

    // if (SIG_WIFI_STATE.resolved && SIG_WIFI_STATE.value == WIFI_STATE_NO_CONFIG)
    // {
    //     changed = true;
    //     message += "  Wi-Fi reconfigure Needed:\n  Please config network by holding down the round button (for 6 seconds)\n\n";
    //     signal_resolve(&SIG_WIFI_STATE);
    // }
    // if (SIG_WIFI_STATE.resolved && SIG_WIFI_STATE.value == WIFI_STATE_CONNECTING)
    // {
    //     changed = true;
    //     message += "  Wi-Fi Connection in process\n\n";
    //     signal_resolve(&SIG_WIFI_STATE);
    // }
    // if (SIG_WIFI_STATE.resolved && SIG_WIFI_STATE.value == WIFI_STATE_FAILED)
    // {
    //     changed = true;
    //     message += "  Wi-Fi Connection Failed\n\n";
    //     message += "  Reconfigure Needed:\n  Please config network by holding down the round button (for 6 seconds)\n\n";
    //     signal_resolve(&SIG_WIFI_STATE);
    // }
    // if (SIG_WIFI_STATE.resolved && SIG_WIFI_STATE.value == WIFI_STATE_NO_MORE_TRY)
    // {
    //     changed = true;
    //     message += "  Wi-Fi reconfigure Needed:\n  Please config network by holding down the round button (for 6 seconds)\n\n";
    //     signal_resolve(&SIG_WIFI_STATE);
    // }
    // if (SIG_WIFI_STATE.resolved && SIG_WIFI_STATE.value == WIFI_STATE_CONNECTED)
    // {
    //     changed = true;
    //     message += "  Wi-Fi is Connected :)\n\n";
    //     signal_resolve(&SIG_WIFI_STATE);
    // }
    // if (SIG_PORTAL_STATE.resolved && SIG_PORTAL_STATE.value > 0)
    // {
    //     changed = true;
    //     message.clear();
    //     message += "  Wi-Fi Configurator is up, Please connect to \n\n";
    //     message += "  [ EM-Paper ]\n\n";
    //     message += "  to setup your device's network\n";
    //     message += "  \n\n";
    //     message += "  This Configurator will stay online for 120 seconds\n\n";
    //     message += "  If you want to cancel, press and hold the button again for 6 seconds.\n\n";
    //     signal_resolve(&SIG_PORTAL_STATE);
    // }
    // if (SIG_PORTAL_STATE.resolved && SIG_PORTAL_STATE.value == 0)
    // {
    //     changed = true;
    //     message.clear();
    //     message += "  Wi-Fi Configurator canceled.\n\n  System shutting down soon.\n\n";
    //     signal_resolve(&SIG_PORTAL_STATE);
    // }

    if (SIG_WAKE.triggered)
    {
        sys_broke = true;
        sig_clear(&SIG_WAKE);
        changed = true;
        message += "  ! Bootstrap Required !\n\n  >> Hold down the knob for 6 seconds to configure Wi-Fi.";
    }
    if (SIG_BEFORE_SLEEP.triggered)
    {
        sig_clear(&SIG_BEFORE_SLEEP);
        changed = true;
        message += "\n\n  [ Powered down ]";
    }
    if (SIG_RTC_INVALID.triggered)
    {
        sig_clear(&SIG_RTC_INVALID);
        changed = true;
        message += "\n\n  ! Time Invalid !\n\n  Wi-Fi connection is required to setup time.";
    }
    // if (SIG_APP_UPT_STATE.resolved && SIG_APP_UPT_STATE.value == APP_UPT_STATE_FAILED)
    // {
    //     signal_resolve(&SIG_APP_UPT_STATE);
    //     changed = true;
    //     message += "  App bundle downloader failed\n\n  Remote might be down, or it could be Wi-Fi reasons.";
    // }
    if (sys_broke)
    {
        sig_set(&SIG_SYS_BROKE, 1);
    }
    if (changed)
    {
        display_dbg_print(message);
    }
}

void sig_external_event()
{
    DEBUG("SIG_EXT", "UPDATE TRIGGERED");
    fallback_renderer();
}

void sleep_prevention()
{
    nap_set_enter_sleep_after(SIG_EINK_DRAW.value + 100); //when drawed, don't sleep
}

void reg_sigs()
{
    sig_reg(&SIG_WAKE);
    sig_reg(&SIG_BEFORE_SLEEP);
    sig_reg(&SIG_ENC_DELTA);
    sig_reg(&SIG_ENC_COUNT);
    sig_reg(&SIG_SW_DOWN);
    sig_reg(&SIG_SW_UP);
    sig_reg(&SIG_SW_CLICK);
    sig_reg(&SIG_SW_HOLD);
    sig_reg(&SIG_TOUCH_DOWN);
    sig_reg(&SIG_TOUCH_CLICK);
    sig_reg(&SIG_USER_ACTION);
    sig_reg(&SIG_FS_MSG);
    sig_reg(&SIG_EINK_DRAW);
    sig_reg(&SIG_RTC_INVALID);

    SIG_USER_ACTION.debug_level = -1;
}

//////////////SYS

void sys_wake();
void sys_tick()
{
    sig_tick();
    sleep_prevention();
    //hal_io_tick <-- this should be called manually
}

void sys_init()
{
    reg_sigs();
    sig_init();
    hal_fs_setup();
    hal_io_setup();
    nap_wake();

    //waking up
    sys_wake();
}

//////////////ACTUAL WAKE SEQ

void sys_wake()
{
    //check bootup hold
    while (SIG_SW_DOWN.value == 1)
    {
        if (millis() > 6000)
        {
            // hal_wifi_reconfig();
        }
    }

    if(SIG_RTC_INVALID.value && !net_update_time()) {
        nap_set_sleep_duration(5000);
        nap_try_sleep(true);
    }
}

#endif