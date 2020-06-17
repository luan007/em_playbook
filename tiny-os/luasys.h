#ifndef GUARD_LUASYS
#define GUARD_LUASYS

#include "hardware.h"
#include "display.h"
#include "drawbin.h"
#include <Preferences.h>
#include "Arduino.h"

#define LUA_USE_C89
#include "src/lua/lua/lua.hpp"

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
        preferences.begin((ns).c_str(), false);
        int val = preferences.getInt(key, val);
        preferences.end();
        lua_pushnumber(lua, (lua_Number)val);
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
        power_eink(1);
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
    }

    static int expose_screen_print_complex(lua_State *lua)
    {
        String a = String(luaL_checkstring(lua, 1));
        int16_t x = (luaL_checkinteger(lua, 2));
        int16_t y = (luaL_checkinteger(lua, 3));
        int16_t w = (luaL_checkinteger(lua, 4));
        int16_t h = (luaL_checkinteger(lua, 5));
        String final = "\n\n" + a;
        power_eink(1);
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
    }

    static int expose_serial_print(lua_State *lua)
    {
        String a = String(luaL_checkstring(lua, 1));
        Serial.print("LUA OUTPUT:\t");
        Serial.println(a);
    }

    static int expose_smart_draw(lua_State *lua)
    {
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
        power_eink(1);
        bin_smart_draw(asset.c_str(),
                       _w, _h,
                       srcminx,
                       srcminy,
                       srcmaxx,
                       srcmaxy,
                       dstx,
                       dsty,
                       flush_count);
    }

    static int expose_flush_screen(lua_State *lua)
    {
        int16_t x = (luaL_checkinteger(lua, 1));
        int16_t y = (luaL_checkinteger(lua, 2));
        int16_t w = (luaL_checkinteger(lua, 3));
        int16_t h = (luaL_checkinteger(lua, 4));
        bool partial = (luaL_checkinteger(lua, 5)) > 0 ? true : false;
        power_eink(1);
        bin_flush_screen(x, y, w, h, partial);
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

    lua_shell_inject_function("load_float", (const lua_CFunction)&expose_load_float);
    lua_shell_inject_function("load_int", (const lua_CFunction)&expose_load_int);
    lua_shell_inject_function("load_string", (const lua_CFunction)&expose_load_string);
    lua_shell_inject_function("save_float", (const lua_CFunction)&expose_save_float);
    lua_shell_inject_function("save_int", (const lua_CFunction)&expose_save_int);
    lua_shell_inject_function("save_string", (const lua_CFunction)&expose_save_string);

    lua_shell_inject_function("file_string", (const lua_CFunction)&expose_file_as_string);

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

String lua_shell_run_code(String code)
{
    String result;
    if (luaL_dostring(_state, code.c_str()))
    {
        result += "# lua error:\n" + String(lua_tostring(_state, -1));
        Serial.println("Error");
        Serial.println(result);
        lua_pop(_state, 1);
    }
    return result;
}

//only functions without any pararm,
//as its hard for us to assemble table (at this moment)
String lua_shell_call_func(String name)
{
    String result;
    lua_getglobal(_state, name.c_str());
    if (lua_isfunction(_state, -1))
    {
        lua_pcall(_state, 0, LUA_MULTRET, 0);
        result += "# lua error:\n" + String(lua_tostring(_state, -1));
        lua_pop(_state, 1);
    }
    return result;
}

#endif