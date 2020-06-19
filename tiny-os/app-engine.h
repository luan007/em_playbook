#ifndef GUARD_APP_ENGINE
#define GUARD_APP_ENGINE
#include <ArduinoJson.h>
#include "network.h"
#include "io.h"
#include "display.h"
#include "luasys.h"
#include "presist.h"
#define APP_ROOT "http://192.168.40.68:9898/"

#define UPDATE_VERSION_INTERVAL 30000
long _last_update = 0;

DynamicJsonDocument app_data(2048); //good chunk of memory

//download one app & inflate
int app_mgr_fetch_app(String app)
{
    download_inflate(app, String(APP_ROOT) + "app/" + app);
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
    String versions_url = String(APP_ROOT) + "versions";
    if (download_from_server("/versions", versions_url) != 1)
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
    if (!error)
    {
        for (auto p : root)
        {
            const char *key = p.key().c_str();
            if (app_mgr_is_update_needed(key, p.value().as<JsonObject>()) > 0)
            {
                app_mgr_fetch_app(key);
            }
        }
    }
    else
    {
        Serial.println("malformed version");
        return -2;
    }
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
    JsonObject root = app_data.as<JsonObject>();
    for (auto p : root)
    {
        app_mgr_run_procedure_meta_loaded(String(p.key().c_str()), key, p.value().as<JsonObject>());
    }
}


int app_version_updator()
{
    _last_update = millis();
    app_mgr_load_versions();
    return 0;
}

//pseudo OS loop
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
    return 0;
}

int app_signal()
{
    app_mgr_loop_procedures("signal");
}
#endif