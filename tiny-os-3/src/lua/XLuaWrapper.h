#ifndef XLUA_WRAPPER_H
#define XLUA_WRAPPER_H

#include "Arduino.h"

#define LUA_USE_C89
#include "lua/lua.hpp"

class XLuaWrapper {
  public:
    XLuaWrapper();
    String Lua_dostring(const String constants, const String *script);
    void Lua_register(const String name, const lua_CFunction function);
    lua_State *_state;
};

#endif
