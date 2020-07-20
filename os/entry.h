#ifndef _GUARD_H_SYS
#define _GUARD_H_SYS

#include "hal-fs.h"
#include "hal-display.h"
#include "hal-io.h"
#include "hal-nap.h"
#include "hal-net.h"
#include "hal-ota.h"
#include "hal-blink.h"
#include "shared.h"
#include "app-engine.h"

#define ENABLE_UART false

////////////HELPER

void factory_reset()
{
  sig_set(&SIG_SYS_MSG, 1);
  cfg_flush_store();
  hal_fs_wipe();
  net_wipe();
  nap_set_sleep_duration(1000);
  nap_try_sleep(true);
}

////////////ALL SIGNALS
void fallback_renderer()
{
  static String message;
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

  if (SIG_OTA.triggered)
  {
    sig_clear(&SIG_OTA);
    changed = true;
    message = "   \n\n";
    message += "   OTA Configurator Is Up, Please Relese The Button.\n\n";
    message += "   Connect To [ EM-Paper ] To Update Your Device.\n\n";
    message += "   Open Brower And Enter [ http://192.168.4.1 ] To Upload Your File.\n\n";
    message += "   \n\n";
    message += "   This Configurator Will Stay Online 240 Seconds\n\n";
    message += "   If You Want To Cancel, Press The Button Again\n\n";
  }
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
  if (SIG_NO_MORE_OP.triggered)
  {
    sys_broke = true;
    sig_clear(&SIG_NO_MORE_OP);
    changed = true;
    message += String("\n\n  Empty loop reached, system corrupt.\n\n");
  }
  if (SIG_APP_UPT_STATE.triggered)
  {
    sys_broke = true;
    sig_clear(&SIG_APP_UPT_STATE);
    String sig_app_upt_msg;
    if (SIG_APP_UPT_STATE.value == APP_UPT_STATE_IDLE)
    {
      sig_app_upt_msg = "/";
    }
    else if (SIG_APP_UPT_STATE.value == APP_UPT_STATE_FAILED)
    {
      sig_app_upt_msg = "[X] Failed";
    }
    else if (SIG_APP_UPT_STATE.value == APP_UPT_STATE_WORKING)
    {
      sig_app_upt_msg = "Working..";
    }
    else if (SIG_APP_UPT_STATE.value == APP_UPT_STATE_SUCC)
    {
      sig_app_upt_msg = "Completed!";
    }
    changed = true;
    message += String("\n\n  Application Updator => ") + sig_app_upt_msg;
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
  app_inject_signals();
  app_before_sleep_cleanup();
  fallback_renderer();
}

void reg_vars()
{
  sig_reg(&SIG_OTA);
  sig_reg(&SIG_OTA_REQ);
  sig_reg(&SIG_PWR_USB);
  sig_reg(&SIG_EINK_MEM_ONLY); //<--warning: things related to this one still needs to be tested
  sig_reg(&SIG_DBG_MODE);
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
  sig_reg(&SIG_FS_TOTAL);
  sig_reg(&SIG_FS_USED_COMPUTED);
  sig_reg(&SIG_FS_FREE);
  sig_reg(&SIG_EINK_DRAW);
  sig_reg(&SIG_RTC_INVALID);
  sig_reg(&SIG_SW_PRESSING);
  sig_reg(&SIG_SYS_MSG);
  sig_reg(&SIG_WIFI_TRY);
  sig_reg(&SIG_TIME);
  sig_reg(&SIG_NOTIFY_RELEASE);
  sig_reg(&SIG_ALLOW_LOOP);
  sig_reg(&SIG_NO_MORE_OP);

  sig_reg(&SIG_CONFIG_CHANGED);
  sig_reg(&SIG_APP_SAFE_RENDER);
  sig_reg(&SIG_APP_UPT_STATE);
  sig_reg(&SIG_APP_TAINT);
  sig_reg(&SIG_APP_REFRESH_REQUEST);
  sig_reg(&SIG_APP_TRY);

  sig_reg(&SIG_BAT);
  sig_reg(&SIG_APP_NRUN);
  sig_reg(&SIG_APP_3PT_NUPD);
  sig_reg(&SIG_APP_3PT_NRUN);
  sig_reg(&SIG_APP_NUPD);
  sig_reg(&SIG_UPD_REQ);

  cfg_reg(&CFG_SRV_ROOT);

  schedulers.push_back(&app_schedule_wake);
  schedulers.push_back(&app_third_party_schedule_wake);

  SIG_USER_ACTION.debug_level = -1;
}

//////////////SYS

void sys_wake();
void sys_tick()
{
  sig_tick();
  //hal_io_tick <-- this should be called manually
}

bool FLAG_OTA_PRESSED = false;
void sys_init()
{
  FLAG_OTA_PRESSED = hal_read_ota_mode_entry();
  LED_A_ON;
  LED_B_ON;
  hal_blink_setup();

  REG_CLR_BIT(RTC_CNTL_STATE0_REG, RTC_CNTL_ULP_CP_SLP_TIMER_EN); //stop ULP immediately
  reg_vars();
  sig_init();
  cfg_init();
  hal_fs_setup();
  hal_io_setup();
  nap_wake();
  //waking up
  sys_wake();
}

//////////////ACTUAL WAKE SEQ
bool compute_sleep_preconditions()
{
  if (SIG_DBG_MODE.value > 0)
    return false;
  if (millis() - (unsigned long)SIG_EINK_DRAW.value < 6000)
    return false;
  if (millis() - (unsigned long)SIG_USER_ACTION.value < 6000)
    return false;
  if (SIG_APP_REFRESH_REQUEST.value > 0)
    return false;
  return true;
}

void sys_wake()
{
  //check bootup hold
  wdt_start();
  sig_tick();
  hal_io_loop();

  if (SIG_OTA_REQ.value > 0 || (FLAG_OTA_PRESSED && SIG_SW_PRESSING.value == 1 && SIG_WAKE.value == WAKE_NONE))
  {
    sig_clear(&SIG_OTA_REQ, 0);
    while (SIG_SW_PRESSING.value == 1)
    {
      hal_io_loop();
      if (millis() > 6000) //this should keep most user away
      {
        sig_set(&SIG_OTA, 1);
        if (SIG_SW_PRESSING.value == 1)
        {
          while (SIG_SW_PRESSING.value == 1)
          {
            hal_io_loop();
          }
        }
        ota_config();
        break;
      }
    }
  }
  else if (SIG_SW_PRESSING.value == 1)
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
  else if (SIG_TOUCH_DOWN.value == 1 && SIG_WAKE.value == WAKE_NONE)
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
  else if (SIG_TOUCH_DOWN.value == 3 && SIG_WAKE.value == WAKE_NONE)
  {
    DEBUG("! TOUCH UPON START", "");
    while (SIG_TOUCH_DOWN.value == 3)
    {
      hal_io_loop();
      if (millis() > 3000)
      {
        net_wifi_connect() && app_mgr_upgrade();
        break; //you should not get here
      }
    }
  }

  DEBUG("TIME-CHECK-ERR", String(SIG_RTC_INVALID.value).c_str());
  DEBUG("TIME-CHECKPOINT", String(rtc_unix_time()).c_str());

  if (SIG_RTC_INVALID.value > 0 && net_update_time() <= 0)
  {
    DEBUG("Time Configuration", "Failed - Retry soon");
    nap_set_sleep_duration((SIG_WIFI_TRY.value > 2 || SIG_APP_TRY.value > 2) ? (60 * 60 * 1000) : 5000);
    return nap_try_sleep(true); //end
  }

  if (SIG_SYS_BROKE.value > 0)
  {
    //FIXING DISK (FORMAT)
    //start network and update
    if (net_wifi_connect() < 0 || app_mgr_upgrade() < 0)
    {
      nap_set_sleep_duration(SIG_WIFI_TRY.value > 3 ? (60 * 60 * 1000) : 5000);
      return nap_try_sleep(true);
    }
    else if (SIG_SYS_BROKE.value == 0) //App fixed all broken signals
    {
      //so we're good now
      return nap_try_sleep(true);
    }
    else
    {
      //still broken, do we want to fix?
      nap_set_sleep_duration((60 * 60 * 1000)); //one hour cool down
      return nap_try_sleep(true);
    }
  }
  //now, system is NOT BROKEN anymore, reboot into app loop yo

  //everything is good, lets see why we're here
  if (SIG_WAKE.value == WAKE_TIMER || SIG_WAKE.value == WAKE_NONE)
  {
    //app needs update?
    if (SIG_UPD_REQ.value > 0)
    {
      DEBUG("force update", "BY SIGNAL SUBSYS");
      int _val = SIG_UPD_REQ.value;
      sig_clear(&SIG_UPD_REQ, 0);
      net_wifi_connect() && app_mgr_upgrade(_val == 2);
    }
    else if (rtc_unix_time() > (uint32_t)SIG_APP_NUPD.value)
    {
      if (app_mgr_upgrade_auto_net() < 0)
      {
        return nap_try_sleep(true);
      }
    }
    //app needs refresh?
    //ACTUALLY
    //IF YOU WAKE UP FROM NOTHINGNESS
    //YOU SHOULD REFRESH
    //TODO: Refine following conditions
    // if (rtc_unix_time() > (uint32_t)SIG_APP_NRUN.value)
    // {
    if (app_refresh_inited == 0 || SIG_APP_TAINT.value > 0)
    {
      //init or fix, anyway, less draw call = better battery life
      app_full_refresh(false);
    }
    // }
  }
  else if (SIG_WAKE.value == WAKE_ULP)
  {
    nap_io_from_ulp();
    if (SIG_DBG_MODE.value > 0)
    {
      DEBUG("X  -  -  WARNING  -  -  X\n\n", "DEBUG_MODE_ENABLED");
    }
    //ok UX triggered, tiny loop then
    // app_full_refresh(); //the user is intended to interact - boot the display first
    // no need though
    while (true)
    {
      SIG_DBG_MODE.value = 0; //force for production
      hal_io_loop();          //any draw action should be safe now
      while (ENABLE_UART && Serial.available())
      {
        //read stuff from serial
        char c = Serial.read();
        switch (c)
        {
        case 'D': //start DBG mode
          sig_set(&SIG_DBG_MODE, 1);
          sig_save(true);
          break;
        case 'E': //stop DBG mode
          sig_set(&SIG_DBG_MODE, 0);
          sig_save(true);
          break;
        case 'G': //force refresh
          Serial.printf("NEXT REQ - %d, <- %d\n", SIG_APP_REFRESH_REQUEST.value, millis());
          break;
        case 'R': //force refresh
          sig_set(&SIG_APP_REFRESH_REQUEST, millis());
          break;
        case 'N': //force download
          net_wifi_connect();
          break;
        case '0': //force download
          net_reset();
          break;
        case 'U': //force download
          app_mgr_upgrade();
          break;
        case 'F': //force download
          DEBUG("WARN", "Forcing full update");
          app_mgr_upgrade(true);
          break;
        }
      }

      sig_tick(); //TODO: CHECK THIS
      if ((unsigned long)SIG_APP_REFRESH_REQUEST.value > 0 && millis() > (unsigned long)SIG_APP_REFRESH_REQUEST.value)
      {
        sig_clear(&SIG_APP_REFRESH_REQUEST, 0);
        app_full_refresh(true);
      }
      if (compute_sleep_preconditions())
      {
        break;
      }
      sig_save();
      yield();
    }
  }
  return nap_try_sleep(true); //this should be your end
}

#endif
