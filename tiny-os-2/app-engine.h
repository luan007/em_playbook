#ifndef _GUARD_APP_ENGINE
#define _GUARD_APP_ENGINE
#include <ArduinoJson.h>
#include "defs.h"
#include "hal-network.h"
#include "hal-io.h"
#include "lua-engine.h"
#include "src/presist.h"
//so crude though
#define SLEEP_BITMASK_APP 5

#define APP_UPT_STATE_SUCC 2
#define APP_UPT_STATE_IDLE 0
#define APP_UPT_STATE_WORKING 1
#define APP_UPT_STATE_FAILED -1

SIGNAL(APP_SAFE_RENDER, "as is", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)
SIGNAL(APP_GOOD, "good through timestamp", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, -1)
SIGNAL(APP_UPT_STATE, "updator state", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)
SIGNAL(APP_UPDATOR_REQUEST, "updator REQUEST", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)
CONFIG(APP_DUE, "good duration", 15, "");
DynamicJsonDocument app_data(2048); //good chunk of memory

//download one app & inflate
int app_mgr_fetch_app(String app)
{
    return net_download_then_inflate(app, CFG_SERVER_ROOT.valueString + "app/" + app);
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

//periodic update of app list
int app_mgr_load_versions()
{
    Serial.println("updating app versions");
    String versions_url = CFG_SERVER_ROOT.valueString + "versions";
    if (net_download_from_server("/versions", versions_url) != 1)
    {
        Serial.println("failed to update versions info, giveup");
        return -1;
    }
    //read everything in mem
    app_data.clear();
    File f = USE_FS.open("/versions");
    String json = f.readString();
    DeserializationError error = deserializeJson(app_data, json);
    f.close();
    JsonObject root = app_data.as<JsonObject>();
    int result = 1;
    if (!error)
    {
        for (auto p : root)
        {
            const char *key = p.key().c_str();
            if (app_mgr_is_update_needed(key, p.value().as<JsonObject>()) > 0)
            {
                result = app_mgr_fetch_app(key) > 0 ? result : -1;
            }
        }
        return result;
    }
    else
    {
        return -2;
    }
    return 1;
}

int app_mgr_run_file(String app, String procedure)
{
    String path = String("/") + app + "/" + procedure + ".lua";
    File f = USE_FS.open(path);
    if (!f)
    {
        f.close();
        Serial.println("APP MGR ERROR: App / Procedure not found");
        Serial.println(path);
        return -1;
    }
    String program = f.readString();
    lua_shell_prep();
    lua_shell_run_code(program);
    lua_shell_destroy();
    return 1;
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

void app_sig_register()
{
    config_register(&CFG_APP_DUE);
    signal_register(&SIG_APP_UPT_STATE);
    signal_register(&SIG_APP_GOOD);
    signal_register(&SIG_APP_SAFE_RENDER);
    signal_register(&SIG_APP_UPDATOR_REQUEST);
}

//pseudo OS loop
int _app_full_refresh_gui_ready = 0;
int app_full_refresh()
{
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
    _app_full_refresh_gui_ready = 1;
    return 0;
}

int app_safe_refresh()
{
    // DEBUG("APP", "SAFE REFRESH");
    if (!_app_full_refresh_gui_ready)
    {
        app_full_refresh();
    }
}

int app_inject_signals()
{
    //check for signals with app visibility & unreset
    int trigger = false;
    for (auto i : signals)
    {
        if (i->resolved && (i->visibility & SIGNAL_VIZ_APP) && (i->reported_value != i->value))
        {
            trigger = true;
            i->reported_value = i->value; //no crazy refresh pls
        }
    }
    if (trigger)
    {
        DEBUG("APP", "TRIGGER SIGNAL");
        app_mgr_loop_procedures("signal");
    }
}

void app_updator_loop()
{
    if (SIG_APP_SAFE_RENDER.value > 0)
    {
        app_safe_refresh();
        SIG_APP_SAFE_RENDER.value = 2;
    }
    if (SIG_APP_UPDATOR_REQUEST.value > 0)
    {
        if (SIG_WIFI_STATE.value == WIFI_STATE_CONNECTED)
        {
            //lock
            DEBUG("APP ENGINE", "LOADING START");
            int sleep_flag = SIG_NO_SLEEP.value;
            bitSet(sleep_flag, SLEEP_BITMASK_APP);
            signal_raise(&SIG_NO_SLEEP, sleep_flag);
            signal_raise(&SIG_APP_UPT_STATE, APP_UPT_STATE_WORKING);
            int result = app_mgr_load_versions();
            signal_raise(&SIG_APP_UPDATOR_REQUEST, 0);
            if (result == 1)
            {
                signal_raise(&SIG_APP_UPT_STATE, APP_UPT_STATE_SUCC);
                signal_raise(&SIG_APP_GOOD, r_secs()); //get epochs
                signal_raise(&SIG_SYS_BROKE, 0);
            }
            else
            {
                signal_raise(&SIG_APP_UPT_STATE, APP_UPT_STATE_FAILED);
            }
            sleep_flag = SIG_NO_SLEEP.value;
            bitClear(sleep_flag, SLEEP_BITMASK_APP);
            signal_raise(&SIG_NO_SLEEP, sleep_flag);
        }
        else if (SIG_WIFI_STATE.value == WIFI_STATE_FAILED || SIG_WIFI_STATE.value == WIFI_STATE_NO_CONFIG || SIG_WIFI_STATE.value == WIFI_STATE_NO_MORE_TRY)
        {
            signal_raise(&SIG_APP_GOOD, -1); //need refresh now, wi-fi sucked
        }
        else
        {
            if (SIG_WIFI_REQ.value == 0)
            {
                Serial.println("REQUESTING WI-FI");
                signal_raise(&SIG_WIFI_REQ, millis());
            }
        }
    }
}

#endif