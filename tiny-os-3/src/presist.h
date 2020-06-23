#ifndef H_PRESIST
#define H_PRESIST

#include <Preferences.h>

Preferences __main_pref;

int getInt(const char *key)
{
    __main_pref.begin("main", false);
    int val = __main_pref.getInt(key);
    __main_pref.end();
    return val;
}

int putInt(const char *key, int val)
{
    __main_pref.begin("main", false);
    __main_pref.putInt(key, val);
    __main_pref.end();
    return val;
}

float getFloat(const char *key)
{
    __main_pref.begin("main", false);
    float val = __main_pref.getFloat(key);
    __main_pref.end();
    return val;
}

float putFloat(const char *key, int val)
{
    __main_pref.begin("main", false);
    __main_pref.putFloat(key, val);
    __main_pref.end();
    return val;
}

String getString(const char *key)
{
    __main_pref.begin("main", false);
    String val = __main_pref.getString(key);
    __main_pref.end();
    return val;
}

String putString(const char *key, String val)
{
    __main_pref.begin("main", false);
    __main_pref.putString(key, val);
    __main_pref.end();
    return val;
}

int64_t getLongInt(const char *key)
{
    __main_pref.begin("main", false);
    int64_t val = __main_pref.getLong64(key);
    __main_pref.end();
    return val;
}

int64_t putLongInt(const char *key, int64_t val)
{
    __main_pref.begin("main", false);
    __main_pref.putLong64(key, val);
    __main_pref.end();
    return val;
}

#endif