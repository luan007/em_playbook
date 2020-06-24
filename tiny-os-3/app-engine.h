#ifndef _GUARD_APP_ENGINE
#define _GUARD_APP_ENGINE
#include <ArduinoJson.h>
#include "src/presist.h"
#include "shared.h"
#include "hal-net.h"
#include "hal-fs.h"
#include "hal-io.h"
#include "lua-engine.h"
//so crude though

int app_full_refresh();

#define APP_UPT_STATE_SUCC 2
#define APP_UPT_STATE_IDLE 0
#define APP_UPT_STATE_WORKING 1
#define APP_UPT_STATE_FAILED -1

#define APP_NEXT_RUN_BAD_INTERVAL 300     //10sec - for debug only
#define APP_NEXT_RUN_DEFAULT_INTERVAL 120 //10sec - for debug only
#define APP_NEXT_RUN_DEFAULT_UPDATE 60    //30sec - for debug only
CONFIG(SRV_ROOT, 0, "http://192.168.1.183:9898/")

SIGNAL(APP_TAINT, SIG_NONE, SIG_RUNTIME, 0)
SIGNAL(APP_REFRESH_REQUEST, SIG_NONE, SIG_RUNTIME, 0)
SIGNAL(APP_SAFE_RENDER, SIG_NONE, SIG_RUNTIME, 0)
SIGNAL(APP_UPT_STATE, SIG_ALL, SIG_IMMEDIATE, 0)
SIGNAL(APP_TRY, SIG_NONE, SIG_POWERLOSS, 0)
SIGNAL(APP_NRUN, SIG_NONE, SIG_POWERLOSS, 0) //NEXT RUN
SIGNAL(APP_NUPD, SIG_NONE, SIG_POWERLOSS, 0) //NEXT UPDATE

DynamicJsonDocument app_data(2048); //good chunk of memory

//download one app & inflate
int app_mgr_fetch_app(String app)
{
    return net_download_then_inflate(app, CFG_SRV_ROOT.valueString + "app/" + app);
}

bool __exists(const String &path)
{
    File f = USE_FS.open(path, "r");
    return (f == true);
}

int app_mgr_package_healthy(String app)
{
    if (!__exists(String("/") + app) || !__exists(String("/") + app + "/meta.json"))
    {
        Serial.println(app + " does not exist / malformed");
        return -1;
    }
    return 1;
}

//download package if needed
int app_mgr_is_update_needed(String app, JsonObject new_version)
{
    //exists?
    if (app_mgr_package_healthy(app) <= 0)
    {
        return 1;
    }
    DynamicJsonDocument doc(512); //load from disk
    File f = USE_FS.open(String("/") + app + "/meta.json");
    String json = f.readString();
    Serial.println(json);
    DeserializationError error = deserializeJson(doc, json);
    f.close();
    if (error)
    {
        Serial.println(json);
        Serial.println(app + " malformed json meta file.");
        return 2;
    }
    JsonObject root = doc.as<JsonObject>();
    int o_version = root["version"];
    int n_version = new_version["version"];
    if (o_version != n_version)
    {
        Serial.println(app + " version mismatch");
        return 3;
    }
    //TODO: add routine update requirement
    return -1;
}

inline void _app_upgrade_failed_attempt()
{
    sig_set(&SIG_APP_TRY, SIG_APP_TRY.value + 1);
}

//periodic update of app list
int app_mgr_upgrade()
{
    if (!WiFi.isConnected())
    {
        sig_set(&SIG_APP_UPT_STATE, APP_UPT_STATE_FAILED);
        _app_upgrade_failed_attempt();
        return -9; //no net
    }
    sig_set(&SIG_APP_UPT_STATE, APP_UPT_STATE_WORKING);
    Serial.println("updating app versions");
    String versions_url = CFG_SRV_ROOT.valueString + "versions";
    if (net_download_from_server("/versions", versions_url) != 1)
    {
        sig_set(&SIG_APP_UPT_STATE, APP_UPT_STATE_FAILED);
        Serial.println("failed to update versions info, giveup");
        _app_upgrade_failed_attempt();
        return -1;
    }
    //read everything in mem
    app_data.clear();
    File f = USE_FS.open("/versions");
    String json = f.readString();
    DeserializationError error = deserializeJson(app_data, json);
    f.close();
    JsonObject root = app_data.as<JsonObject>();
    if (error)
    {
        sig_set(&SIG_APP_UPT_STATE, APP_UPT_STATE_FAILED);
        _app_upgrade_failed_attempt();
        return -2; //version parse error
    }
    int result = 1; //up to date
    for (auto p : root)
    {
        const char *key = p.key().c_str();
        if (app_mgr_is_update_needed(key, p.value().as<JsonObject>()) > 0)
        {
            result = 2;                                        //something has change
            result = app_mgr_fetch_app(key) > 0 ? result : -3; //has app error
        }
    }
    if (result < 0)
    {
        _app_upgrade_failed_attempt();
        sig_set(&SIG_APP_UPT_STATE, APP_UPT_STATE_FAILED);
    }
    else
    {
        sig_set(&SIG_APP_NRUN, result == 2 ? 0 : SIG_APP_NRUN.value); //no need if there's no change
        sig_set(&SIG_APP_NUPD, rtc_unix_time() + APP_NEXT_RUN_DEFAULT_UPDATE);
        sig_set(&SIG_APP_TRY, 0);
        sig_set(&SIG_APP_UPT_STATE, APP_UPT_STATE_SUCC);
    }
    _dbg_ls_dir("/", 100);

    //APP UPDATED, rerender must happen
    if (result == 2)
    {
        app_full_refresh();
    }
    return result;
}

int app_mgr_upgrade_auto_net()
{
    if (net_wifi_connect() < 0)
    {
        _app_upgrade_failed_attempt();
        return -1;
    }
    return app_mgr_upgrade();
}

int app_mgr_run_file(String app, String procedure)
{
    String path = String("/") + app + "/" + procedure + ".lua";
    File f = USE_FS.open(path);
    int result = 1;
    if (!f)
    {
        f.close();
        Serial.println("APP MGR ERROR: App / Procedure not found");
        Serial.println(path);
        result = -1;
    }
    else
    {
        String program = f.readString();
        lua_shell_prep();
        if (lua_shell_run_code(program) < 0)
        {
            result = -2;
        }
        lua_shell_destroy();
    }
    if (result < 0)
    {
        //ERROR!
        //clean up folder & mark broken
        //app crashed, might need recovery
        _rm_recur((String("/") + app).c_str());
        sig_set(&SIG_SYS_BROKE, 1);
        sig_set(&SIG_APP_NRUN, 0);
    }
    return result;
}

int app_mgr_run_procedure_meta_loaded(String app, String key, JsonObject root)
{
    if (!root.containsKey("capability"))
    {
        return -3;
    }
    JsonObject caps = root["capability"].as<JsonObject>();
    if (!caps.containsKey(key))
    {
        return -4;
    }
    String target = caps[key].as<String>();
    return app_mgr_run_file(app, target);
}

int app_mgr_run_procedure(String app, String key)
{
    if (app_mgr_package_healthy(app) <= 0)
    {
        return -1;
    }
    DynamicJsonDocument doc(512); //load from disk
    File f = USE_FS.open(String("/") + app + "/meta.json");
    String json = f.readString();
    DeserializationError error = deserializeJson(doc, json);
    f.close();
    if (error)
    {
        Serial.println(json);
        Serial.println(app + " malformed json meta file.");
        return -2;
    }
    JsonObject root = doc.as<JsonObject>();
    return app_mgr_run_procedure_meta_loaded(app, key, root);
}

int app_mgr_loop_procedures(String key)
{
    Serial.print("APP LOOP Procedures - ");
    Serial.println(key);
    app_data.clear();
    File f = USE_FS.open("/versions");
    String json = f.readString();
    DeserializationError error = deserializeJson(app_data, json);
    f.close();
    if (error)
    {
        return -1;
    }
    JsonObject root = app_data.as<JsonObject>();
    for (auto p : root)
    {
        app_mgr_run_procedure_meta_loaded(String(p.key().c_str()), key, p.value().as<JsonObject>());
    }
}

//pseudo OS loop
int app_refresh_inited = 0;

int app_full_refresh()
{
    sig_clear(&SIG_APP_TAINT, 0);
    String current_app = getString("APP");
    if (current_app != "")
    {
        app_mgr_run_procedure(getString("APP"), "main");
    }
    else
    {
        app_mgr_run_procedure("os", "main");
    }
    app_mgr_loop_procedures("os");
    app_mgr_loop_procedures("overlay");
    app_refresh_inited = 1;
    sig_set(&SIG_APP_NRUN, rtc_unix_time() + APP_NEXT_RUN_DEFAULT_INTERVAL);
    return 0;
}

int app_safe_refresh()
{
    if (!app_refresh_inited)
    {
        DEBUG("APP", "SAFE REFRESH");
        app_full_refresh();
    }
}

int app_inject_signals()
{
    // //check for signals with app visibility & unreset
    // int trigger = false;
    // for (auto i : signals)
    // {
    //     if (i->triggered && (i->visibility & SIG_APP) && (i->_notified_value != i->value))
    //     {
    //         trigger = true;
    //         i->_notified_value = i->value; //no crazy refresh pls
    //     }
    // }
    // if (trigger)
    // {
    Serial.println("APP SIG TRIGGER!----");
    app_mgr_loop_procedures("signal");
    // }
}

void app_loop()
{
    app_inject_signals(); //DO WE HAVE ANY NEWS?

    int bypass = 0;
    if (SIG_BEFORE_SLEEP.value == 1)
    {
        if (SIG_APP_TAINT.value > 0)
        {
            //immediate refresh requried
            app_full_refresh();
            bypass = 1;
        }
    }
    else if (SIG_APP_REFRESH_REQUEST.value > 0 && millis() > SIG_APP_REFRESH_REQUEST.value)
    {
        app_full_refresh();
        sig_clear(&SIG_APP_REFRESH_REQUEST, 0);
    }
}

uint32_t app_schedule_wake()
{

    uint32_t next_wake_point = SIG_APP_NUPD.value;

    if (SIG_APP_TRY.value > 3) //give several tries
    {
        //we wont touch APP_NUPD, but just override by code
        next_wake_point = ((uint32_t)rtc_unix_time() + (uint32_t)APP_NEXT_RUN_BAD_INTERVAL); //after 100s
        sig_set(&SIG_APP_TRY, 0);                                                            //reset
    }

    uint32_t now = rtc_unix_time();
    if ((uint32_t)SIG_APP_NRUN.value < next_wake_point)
    {
        next_wake_point = SIG_APP_NRUN.value;
    }
    if (next_wake_point > now)
    {
        next_wake_point -= now;
    }
    else
    {
        next_wake_point = 0;
    }
    next_wake_point *= (uint32_t)1000;
    if (next_wake_point == 0)
    {
        next_wake_point += 1000; //cool down period
    }
    DEBUG("APP SCHEDULE WAKE", String(next_wake_point).c_str());
    return next_wake_point;
}

#endif