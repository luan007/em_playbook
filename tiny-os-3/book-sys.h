#ifndef _GUARD_H_SYS
#define _GUARD_H_SYS

#include "hal-fs.h"
#include "hal-display.h"
#include "hal-io.h"
#include "hal-nap.h"
#include "hal-net.h"
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

    if (SIG_SYS_MSG.triggered)
    {
        sys_broke = true;
        message += "\n\n  [ SYSTEM WIPE ]\n\n\n\n  IN PROCESS\n\n";
        sig_clear(&SIG_SYS_MSG);
        changed = true;
    }
    if (SIG_WAKE.triggered)
    {
        sys_broke = true;
        sig_clear(&SIG_WAKE);
        changed = true;
        message += "\n\n  ! Bootstrap Required !\n\n  >> Hold down the knob for 6 seconds to configure Wi-Fi.\n\n";
    }
    if (SIG_BEFORE_SLEEP.triggered)
    {
        sig_clear(&SIG_BEFORE_SLEEP);
        changed = true;
        message += "\n\n  [ Powered down ]";
        message += String("\n\n  Wake Scheduled = ") + SIG_WAKE_AFTER.value + "\n\n";
    }
    if (SIG_RTC_INVALID.triggered)
    {
        sys_broke = true;
        sig_clear(&SIG_RTC_INVALID);
        changed = true;
        message += "\n\n  ! Time Invalid !\n\n  Wi-Fi connection is required to setup time.\n\n";
    }
    if (SIG_WIFI.triggered)
    {
        sys_broke = true;
        sig_clear(&SIG_WIFI);
        changed = true;
        message += String("\n\n  WIFI STATE = ") + SIG_WIFI.value;
    }
    if (SIG_NOTIFY_RELEASE.triggered)
    {
        message.clear();
        sys_broke = true;
        sig_clear(&SIG_NOTIFY_RELEASE);
        changed = true;
        message += String("\n\n  Release button to continue.\n\n");
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
    sig_reg(&SIG_WIFI);
    sig_reg(&SIG_BEFORE_SLEEP);
    sig_reg(&SIG_WAKE_AFTER);
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
    sig_reg(&SIG_SW_PRESSING);
    sig_reg(&SIG_SYS_MSG);
    sig_reg(&SIG_WIFI_TRY);
    sig_reg(&SIG_TIME);
    sig_reg(&SIG_NOTIFY_RELEASE);

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
    REG_CLR_BIT(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN); //stop ULP immediately
    reg_sigs();
    sig_init();
    hal_fs_setup();
    hal_io_setup();
    nap_wake();
    //waking up
    sys_wake();
}

void factory_reset()
{
    sig_set(&SIG_SYS_MSG, 1);
    hal_fs_wipe();
    net_wipe();
    nap_set_sleep_duration(1000);
    nap_try_sleep(true);
}

//////////////ACTUAL WAKE SEQ

void sys_wake()
{
    //check bootup hold
    wdt_start();
    sig_tick();
    hal_io_loop();

    if (SIG_SW_PRESSING.value == 1)
    {
        DEBUG("! PRESS UPON START", "");
        while (SIG_SW_PRESSING.value == 1)
        {
            hal_io_loop();
            if (millis() > 6000)
            {
                if (SIG_SW_PRESSING.value == 1)
                {
                    DEBUG("! STILL PRESSING AFTER 6000", "");
                    sig_set(&SIG_NOTIFY_RELEASE, 1);
                    while (SIG_SW_PRESSING.value == 1)
                    {
                        hal_io_loop();
                    }
                }
                // hal_wifi_reconfig();
                net_wifi_config(); //after this, wifi-config done
                break;             //you should not get here
            }
        }
    }
    else if (SIG_TOUCH_DOWN.value == 1)
    {
        DEBUG("! TOUCH UPON START", "");
        while (SIG_TOUCH_DOWN.value == 1)
        {
            hal_io_loop();
            if (millis() > 6000)
            {
                // hal_wifi_reconfig();
                factory_reset(); //after this, wifi-config done
                break;           //you should not get here
            }
        }
    }

    DEBUG("TIME-CHECK-ERR", String(SIG_RTC_INVALID.value).c_str());

    if (SIG_RTC_INVALID.value > 0 && net_update_time() <= 0)
    {
        DEBUG("Time Configuration", "Failed - Retry soon");
        nap_set_sleep_duration(SIG_WIFI_TRY.value > 3 ? (60 * 60 * 1000) : 10000);
        nap_try_sleep(true);
    }
}

#endif