#ifndef GUARD_LUASYS
#define GUARD_LUASYS

#include <Preferences.h>
#include "Arduino.h"
#include "shared.h"
#include "hal-io.h"
#include "hal-display.h"
#include "./src/presist.h"

#define LUA_USE_C89
#include "src/lua/lua/lua.hpp"

// extern int app_full_refresh();
// extern int app_safe_refresh();

extern int app_restore_display_memory();

String lua_shell_injection; //code pieces gets called before anything

void lua_set_shell_reason(const char *reason)
{
    // Serial.print("REASON PREP");
    // Serial.println(reason);
    lua_shell_injection.clear();
    lua_shell_injection = String("REASON = \"") + reason + "\"";
}

Preferences preferences;
extern "C"
{

    static int expose_file_as_string(lua_State *lua)
    {
        File f = USE_FS.open(luaL_checkstring(lua, 1));
        String json = f.readString();
        lua_pushstring(lua, json.c_str());
        return 1;
    }

    static int expose_save_int(lua_State *lua)
    {
        //namespace, key, value
        String ns = String(luaL_checkstring(lua, 1));
        const char *key = (luaL_checkstring(lua, 2));
        int val = (luaL_checkinteger(lua, 3));
        preferences.begin((ns).c_str(), false);
        preferences.putInt(key, val);
        preferences.end();
    }

    static int expose_save_float(lua_State *lua)
    {
        //namespace, key, value
        String ns = String(luaL_checkstring(lua, 1));
        const char *key = (luaL_checkstring(lua, 2));
        float val = (luaL_checknumber(lua, 3));
        preferences.begin((ns).c_str(), false);
        preferences.putFloat(key, val);
        preferences.end();
    }

    static int expose_save_string(lua_State *lua)
    {
        //namespace, key, value
        String ns = String(luaL_checkstring(lua, 1));
        const char *key = (luaL_checkstring(lua, 2));
        String val = String(luaL_checkstring(lua, 3));
        preferences.begin((ns).c_str(), false);
        preferences.putString(key, val);
        preferences.end();
    }

    static int expose_load_int(lua_State *lua)
    {
        //namespace, key, value
        String ns = String(luaL_checkstring(lua, 1));
        const char *key = (luaL_checkstring(lua, 2));
        int def = 0;
        if (lua_gettop(lua) == 3)
        {
            def = luaL_checkinteger(lua, 3);
        }
        preferences.begin((ns).c_str(), false);
        int val = preferences.getInt(key, def);
        preferences.end();
        lua_pushnumber(lua, (lua_Number)val);
        return 1;
    }

    static int expose_set_signal_value(lua_State *lua)
    {
        String ns = String(luaL_checkstring(lua, 1));
        int val = (luaL_checkinteger(lua, 2));
        sig_set(ns.c_str(), val);
        return 1;
    }

    static int expose_resolve_signal(lua_State *lua)
    {
        String ns = String(luaL_checkstring(lua, 1));
        auto sig = sig_find(ns.c_str());
        if (sig != NULL)
        {
            if (lua_gettop(lua) == 2)
            {
                auto num = luaL_checkinteger(lua, 2);
                sig_clear(sig, num);
            }
            else
            {
                sig_clear(sig);
            }
        }
        return 1;
    }

    static int expose_is_signal_raised(lua_State *lua)
    {
        String ns = String(luaL_checkstring(lua, 1));
        auto sig = sig_find(ns.c_str());
        lua_pushnumber(lua, sig->triggered);
        return 1;
    }

    static int expose_get_signal_value(lua_State *lua)
    {
        String ns = String(luaL_checkstring(lua, 1));
        auto sig = sig_find(ns.c_str());
        lua_pushnumber(lua, sig->value);
        return 1;
    }

    static int expose_load_float(lua_State *lua)
    {
        //namespace, key, value
        String ns = String(luaL_checkstring(lua, 1));
        const char *key = (luaL_checkstring(lua, 2));
        preferences.begin((ns).c_str(), false);
        float val = preferences.getFloat(key);
        preferences.end();
        lua_pushnumber(lua, (lua_Number)val);
        return 1;
    }

    static int expose_load_string(lua_State *lua)
    {
        //namespace, key, value
        String ns = String(luaL_checkstring(lua, 1));
        const char *key = (luaL_checkstring(lua, 2));
        preferences.begin((ns).c_str(), false);
        String val = preferences.getString(key);
        preferences.end();
        lua_pushstring(lua, val.c_str());
        return 1;
    }

    static int expose_gpio_pinmode(lua_State *lua)
    {
        int16_t io = (luaL_checkinteger(lua, 1));
        int16_t mode = (luaL_checkinteger(lua, 2));
        pinMode(io, mode);
    }

    static int expose_gpio_write(lua_State *lua)
    {
        int16_t io = (luaL_checkinteger(lua, 1));
        int16_t mode = (luaL_checkinteger(lua, 2));
        digitalWrite(io, mode);
    }

    static int expose_millis(lua_State *lua)
    {
        lua_pushnumber(lua, (lua_Number)millis());
        return 1;
    }

    static int expose_unix_time(lua_State *lua)
    {
        lua_pushnumber(lua, (lua_Number)rtc_unix_time());
        return 1;
    }

    static int expose_now(lua_State *lua)
    {
        auto dt = rtc_date_time();
        lua_pushnumber(lua, (lua_Number)dt.year());
        lua_pushnumber(lua, (lua_Number)dt.month());
        lua_pushnumber(lua, (lua_Number)dt.day());
        lua_pushnumber(lua, (lua_Number)dt.dayOfTheWeek());
        lua_pushnumber(lua, (lua_Number)dt.hour());
        lua_pushnumber(lua, (lua_Number)dt.minute());
        lua_pushnumber(lua, (lua_Number)dt.second());
        return 7;
    }

    static int expose_appoint_reboot_sec(lua_State *lua)
    {
        int val_secs = (luaL_checkinteger(lua, 1));
        if (SIG_APP_3PT_NUPD.value == 0 || SIG_APP_3PT_NUPD.value > val_secs)
        {
            sig_set(&SIG_APP_3PT_NUPD, val_secs);
        }
        return 1;
    }

    static int expose_micros(lua_State *lua)
    {
        lua_pushnumber(lua, (lua_Number)micros());
        return 1;
    }

    static int expose_delay(lua_State *lua)
    {
        int16_t d = (luaL_checkinteger(lua, 1));
        delay(d);
        return 1;
    }

    static int expose_yield(lua_State *lua)
    {
        yield();
        return 1;
    }

    static int expose_screen_print_simple(lua_State *lua)
    {
        String a = String(luaL_checkstring(lua, 1));
        String final = "\n\n" + a;
        display_power(1);
        display.setRotation(0);
        display.setPartialWindow(0, 0, 800, 300);
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.firstPage();
        do
        {
            display.setCursor(0, 0);
            display.fillScreen(GxEPD_WHITE);
            display.println(final);
        } while (display.nextPage());
        return 1;
    }

    static int expose_screen_print_complex(lua_State *lua)
    {
        String a = String(luaL_checkstring(lua, 1));
        int16_t x = (luaL_checkinteger(lua, 2));
        int16_t y = (luaL_checkinteger(lua, 3));
        int16_t w = (luaL_checkinteger(lua, 4));
        int16_t h = (luaL_checkinteger(lua, 5));
        String final = "\n\n" + a;
        display_power(1);
        display.setRotation(0);
        display.setPartialWindow(x, y, w, h);
        display.setFont(&FreeMonoBold9pt7b);
        display.setTextColor(GxEPD_BLACK);
        display.firstPage();
        do
        {
            display.setCursor(0, 0);
            display.fillScreen(GxEPD_WHITE);
            display.println(final);
        } while (display.nextPage());
        return 1;
    }

    static int expose_serial_print(lua_State *lua)
    {
        String a = String(luaL_checkstring(lua, 1));
        Serial.print("LUA OUTPUT:\t");
        Serial.println(a);
        return 1;
    }

    static int expose_load_lib(lua_State *lua)
    {
        String path = String(luaL_checkstring(lua, 1));
        //note this one is not isolated yet
        //TODO: add security
        File file = USE_FS.open(path);
        const int ret = luaL_dostring(lua, file.readString().c_str());
        if (ret != LUA_OK)
        {
            Serial.println("Error\n");
            Serial.println(lua_tostring(lua, -1));
            lua_pop(lua, 1); // pop error message
        }
        file.close();
        return 1;
    }

    static int expose_smart_draw(lua_State *lua)
    {
        app_restore_display_memory();
        String asset = String(luaL_checkstring(lua, 1));
        int16_t _w = (luaL_checkinteger(lua, 2));
        int16_t _h = (luaL_checkinteger(lua, 3));
        int16_t srcminx = (luaL_checkinteger(lua, 4));
        int16_t srcminy = (luaL_checkinteger(lua, 5));
        int16_t srcmaxx = (luaL_checkinteger(lua, 6));
        int16_t srcmaxy = (luaL_checkinteger(lua, 7));
        int16_t dstx = (luaL_checkinteger(lua, 8));
        int16_t dsty = (luaL_checkinteger(lua, 9));
        int16_t flush_count = (luaL_checkinteger(lua, 10));
        display_power(1);
        display_bin_smart_draw(asset.c_str(),
                               _w, _h,
                               srcminx,
                               srcminy,
                               srcmaxx,
                               srcmaxy,
                               dstx,
                               dsty,
                               flush_count);
        return 1;
    }

    static int expose_smart_draw_corrected_dim(lua_State *lua)
    {
        app_restore_display_memory();
        String asset = String(luaL_checkstring(lua, 1));
        //note, twist
        int16_t imgw = (luaL_checkinteger(lua, 2));
        int16_t imgh = (luaL_checkinteger(lua, 3));
        int16_t srcminx = (luaL_checkinteger(lua, 4));
        int16_t srcminy = (luaL_checkinteger(lua, 5));
        int16_t srcmaxx = (luaL_checkinteger(lua, 6));
        int16_t srcmaxy = (luaL_checkinteger(lua, 7));
        int16_t dstx = (luaL_checkinteger(lua, 8));
        int16_t dsty = (luaL_checkinteger(lua, 9));
        int16_t flush_count = (luaL_checkinteger(lua, 10));
        int16_t width = srcmaxx - srcminx;

        display_power(1);
        display_bin_smart_draw(asset.c_str(),
                               imgh, imgw,
                               srcminy,        //real x on flipped img
                               imgw - srcmaxx, //real y on flipped img
                               srcmaxy,
                               imgw - srcminx,
                               dsty,
                               600 - dstx - width,
                               flush_count);
        return 1;
    }

    static int expose_smart_draw_corrected_relative(lua_State *lua)
    {
        app_restore_display_memory();
        String asset = String(luaL_checkstring(lua, 1));
        //note, twist
        int16_t imgw = (luaL_checkinteger(lua, 2));
        int16_t imgh = (luaL_checkinteger(lua, 3));
        int16_t srcminx = (luaL_checkinteger(lua, 4));
        int16_t srcminy = (luaL_checkinteger(lua, 5));
        int16_t srcmaxx = srcminx + (luaL_checkinteger(lua, 6));
        int16_t srcmaxy = srcminy + (luaL_checkinteger(lua, 7));
        int16_t dstx = (luaL_checkinteger(lua, 8));
        int16_t dsty = (luaL_checkinteger(lua, 9));
        int16_t flush_count = (luaL_checkinteger(lua, 10));
        srcmaxx = srcmaxx > imgw ? imgw : srcmaxx;
        srcmaxy = srcmaxy > imgh ? imgh : srcmaxy;
        int16_t width = srcmaxx - srcminx;
        display_power(1);
        display_bin_smart_draw(asset.c_str(),
                               imgh, imgw,
                               srcminy,        //real x on flipped img
                               imgw - srcmaxx, //real y on flipped img
                               srcmaxy,
                               imgw - srcminx,
                               dsty,
                               600 - dstx - width,
                               flush_count);
        return 1;
    }

    static int expose_flush_screen(lua_State *lua)
    {
        app_restore_display_memory();
        int16_t x = (luaL_checkinteger(lua, 1));
        int16_t y = (luaL_checkinteger(lua, 2));
        int16_t w = (luaL_checkinteger(lua, 3));
        int16_t h = (luaL_checkinteger(lua, 4));
        bool partial = (luaL_checkinteger(lua, 5)) > 0 ? true : false;
        display_power(1);
        display_bin_flush_screen(x, y, w, h, partial);
        return 1;
    }

    static int expose_flush_screen_corrected(lua_State *lua)
    {
        app_restore_display_memory();
        int16_t x = (luaL_checkinteger(lua, 1));
        int16_t y = (luaL_checkinteger(lua, 2));
        int16_t w = (luaL_checkinteger(lua, 3));
        int16_t h = (luaL_checkinteger(lua, 4));
        bool partial = (luaL_checkinteger(lua, 5)) > 0 ? true : false;
        display_power(1);
        display_bin_flush_screen(y, x, h, w, partial);
        return 1;
    }

    static int expose_flush_auto(lua_State *lua)
    {
        app_restore_display_memory();
        display_power(1);
        display_bin_auto_flush_if_dirty(
            luaL_checkinteger(lua, 1),
            luaL_checkinteger(lua, 2) > 0 ? true : false);
        return 1;
    }

    static int expose_flush_clear(lua_State *lua)
    {
        reset_dirty_indicator();
        return 1;
    }

    static int expose_require_base_render(lua_State *lua)
    {
        app_restore_display_memory();
        return 1;
    }

    static int expose_screen_need_restore(lua_State *lua)
    {
        sig_set(&SIG_APP_TAINT, 1);
        if (lua_gettop(lua) == 1)
        {
            int16_t auto_restore = (luaL_checkinteger(lua, 1));
            sig_set(&SIG_APP_REFRESH_REQUEST,
                    SIG_APP_REFRESH_REQUEST.value > auto_restore ? auto_restore : SIG_APP_REFRESH_REQUEST.value);
        }
        return 1;
    }
}

lua_State *_state;

//https://github.com/sfranzyshen/ESP-Arduino-Lua
//   Serial.println(lua_wrapper.Lua_dostring(&script));
// LuaWrapper lua_wrapper;

// static int exposed_draw_string(lua_State *lua)
// {
//     String a = String(luaL_checkstring(lua, 1));
//     draw_text(a);
// }
// String script = String("draw_string(\"hello there from lua\")");

void setup_lua()
{
    // lua_wrapper.Lua_register("draw_string", (const lua_CFunction)&exposed_draw_string);
}
void lua_shell_inject_function(const String name, const lua_CFunction function)
{
    lua_register(_state, name.c_str(), function);
}

void lua_shell_prep()
{
    _state = luaL_newstate();
    luaL_requiref(_state, "", luaopen_base, 1);
    lua_pop(_state, 1);
    luaL_requiref(_state, LUA_TABLIBNAME, luaopen_table, 1);
    lua_pop(_state, 1);
    luaL_requiref(_state, LUA_STRLIBNAME, luaopen_string, 1);
    lua_pop(_state, 1);
    luaL_requiref(_state, LUA_DBLIBNAME, luaopen_debug, 1);
    lua_pop(_state, 1);
    luaL_requiref(_state, LUA_MATHLIBNAME, luaopen_math, 1);
    lua_pop(_state, 1);

    lua_shell_inject_function("loadlib", (const lua_CFunction)&expose_load_lib);
    lua_shell_inject_function("sprint", (const lua_CFunction)&expose_serial_print);
    lua_shell_inject_function("delay", (const lua_CFunction)&expose_delay);
    lua_shell_inject_function("pinMode", (const lua_CFunction)&expose_gpio_pinmode);
    lua_shell_inject_function("digitalWrite", (const lua_CFunction)&expose_gpio_write);
    lua_shell_inject_function("micros", (const lua_CFunction)&expose_micros);
    lua_shell_inject_function("millis", (const lua_CFunction)&expose_millis);
    lua_shell_inject_function("complex_log", (const lua_CFunction)&expose_screen_print_complex);
    lua_shell_inject_function("simple_log", (const lua_CFunction)&expose_screen_print_simple);
    lua_shell_inject_function("smart_draw", (const lua_CFunction)&expose_smart_draw);
    lua_shell_inject_function("sys_yield", (const lua_CFunction)&expose_yield);
    lua_shell_inject_function("flush_screen", (const lua_CFunction)&expose_flush_screen);
    lua_shell_inject_function("flush_screen_c", (const lua_CFunction)&expose_flush_screen_corrected);
    lua_shell_inject_function("flush_auto", (const lua_CFunction)&expose_flush_auto);
    lua_shell_inject_function("clear_auto_flush", (const lua_CFunction)&expose_flush_clear);
    lua_shell_inject_function("smart_draw_c", (const lua_CFunction)&expose_smart_draw_corrected_dim);
    lua_shell_inject_function("smart_draw_r", (const lua_CFunction)&expose_smart_draw_corrected_relative);
    lua_shell_inject_function("unix", (const lua_CFunction)&expose_unix_time);
    lua_shell_inject_function("now", (const lua_CFunction)&expose_now);
    lua_shell_inject_function("appoint", (const lua_CFunction)&expose_appoint_reboot_sec);

    lua_shell_inject_function("load_float", (const lua_CFunction)&expose_load_float);
    lua_shell_inject_function("load_int", (const lua_CFunction)&expose_load_int);
    lua_shell_inject_function("load_string", (const lua_CFunction)&expose_load_string);
    lua_shell_inject_function("save_float", (const lua_CFunction)&expose_save_float);
    lua_shell_inject_function("save_int", (const lua_CFunction)&expose_save_int);
    lua_shell_inject_function("save_string", (const lua_CFunction)&expose_save_string);

    lua_shell_inject_function("file_string", (const lua_CFunction)&expose_file_as_string);

    lua_shell_inject_function("sig_alert", (const lua_CFunction)&expose_is_signal_raised);
    lua_shell_inject_function("sig_get", (const lua_CFunction)&expose_get_signal_value);
    lua_shell_inject_function("sig_set", (const lua_CFunction)&expose_set_signal_value);
    lua_shell_inject_function("sig_clear", (const lua_CFunction)&expose_resolve_signal);
    lua_shell_inject_function("mem_draw", (const lua_CFunction)&expose_require_base_render);
    lua_shell_inject_function("req_redraw", (const lua_CFunction)&expose_screen_need_restore);
    lua_shell_inject_function("tainted", (const lua_CFunction)&expose_screen_need_restore);

    const int ret = luaL_dostring(_state, lua_shell_injection.c_str());
    Serial.println(lua_shell_injection);
    if (ret != LUA_OK)
    {
        Serial.println("Error\n");
        Serial.println(lua_tostring(_state, -1));
        lua_pop(_state, 1); // pop error message
    }

    //inject libs
    File shared_root = USE_FS.open("/shared");
    if (!shared_root.isDirectory())
    {
        Serial.println("Shared is not a directory, many things might not work");
        return;
    }

    File file = shared_root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            //skip
        }
        else
        {
            if (String(file.name()).endsWith(".lua"))
            {
                Serial.print("LOAD LIB: ");
                Serial.print(file.name());
                Serial.print("\tSIZE: ");
                Serial.println(file.size());
                const int ret = luaL_dostring(_state, file.readString().c_str());
                if (ret != LUA_OK)
                {
                    Serial.println("Error\n");
                    Serial.println(lua_tostring(_state, -1));
                    lua_pop(_state, 1); // pop error message
                }
            }
        }
        file = shared_root.openNextFile();
    }
}

void lua_shell_destroy() //this should be it
{
    lua_close(_state);
}

int lua_shell_run_code(String code)
{
    String result;
    int res = 1;
    if (luaL_dostring(_state, code.c_str()))
    {
        result += "# lua error:\n" + String(lua_tostring(_state, -1));
        Serial.println("Error");
        Serial.println(result);
        lua_pop(_state, 1);
        res = -1;
    }
    return res;
}

//only functions without any pararm,
//as its hard for us to assemble table (at this moment)
int lua_shell_call_func(String name)
{
    String result;
    int res = 1;
    lua_getglobal(_state, name.c_str());
    if (lua_isfunction(_state, -1))
    {
        lua_pcall(_state, 0, LUA_MULTRET, 0);
        result += "# lua error:\n" + String(lua_tostring(_state, -1));
        res = -1;
        lua_pop(_state, 1);
    }
    else
    {
        res = -2;
    }
    return res;
}

#endif