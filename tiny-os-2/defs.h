#ifndef _GUARD_H_DEF
#define _GUARD_H_DEF
//TODO: note, wear levelling should be considered
#include "time.h"
#include <list>
#include <Preferences.h>

#define DBG_LEVEL_LOG 0

void DEBUG(const char *module, const char *stuff, int LEVEL = DBG_LEVEL_LOG)
{
    auto msg = String("[DBG] ") + module + " : " + stuff;
    Serial.println(msg.c_str());
}

int64_t epochs()
{
    return static_cast<int64_t>(time(NULL));
}

int64_t r_millis() //trustworthy time!
{
    return static_cast<int64_t>(time(NULL));
}

typedef struct schedule
{
    long next_wake; //current time = 0, computed = internal delta + current
    void (*compute)();
    const char *name;
};

std::list<struct schedule *> schedules;

void schedule_register(struct schedule *sch)
{
    schedules.push_back(sch);
}

struct schedule *schedule_get(const char *name)
{
    for (auto i : schedules)
    {
        if (strcmp(i->name, name) == 0)
        {
            return i;
        }
    }
    return NULL;
}

void schedule_recompute_all()
{
    for (auto i : schedules)
    {
        if (i->compute != NULL)
        {
            i->compute();
        }
    }
}

//this is the worst, things are broken, errmsg must be printed
#define SIGNAL_VIZ_IMMEDIATE_FALLBACK (1 << 4)
//if not resolved, this msg will be shown as text
#define SIGNAL_VIZ_USER (1 << 3) //
//will relay this signal to app
#define SIGNAL_VIZ_APP (1 << 2)
//used inside os, it alone means app will not get this, nor the user
#define SIGNAL_VIZ_OS (1 << 1)

#define SIGNAL_VIZ_NONE 0
#define SIGNAL_VIZ_ALL 0xFFFF

#define SIGNAL_RAISED 1
#define SIGNAL_RESOLVED 0

#define SIGNAL_PRESIST_ONCE 1
#define SIGNAL_PRESIST_RUNTIME 0
#define SIGNAL_PRESIST_POWERLOSS 2
#define SIGNAL_PRESIST_ONCE_AUTO_ZERO 3

Preferences _signal_store;
typedef struct signal
{
    const char *name;
    const char *fallback_msg;
    int visibility;
    int presist_behavior;
    int resolved;
    int64_t value;
    int _saved_value; //to ensure not writing too many times during loop
};

#define SIGNAL(NAME, default_msg, visibility, presist_behavior, default_value) struct signal SIG_##NAME = {#NAME, default_msg, visibility, presist_behavior, 0, default_value};

SIGNAL(FLUSH_SIGS, "Flush Store", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, 0)
SIGNAL(FLUSH_CONFIG, "Flush Config", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, 0)
SIGNAL(CONFIG_CHANGED, "Config Changed", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_ONCE_AUTO_ZERO, 0)

std::list<struct signal *> signals;

void signal_register(struct signal *sig)
{
    signals.push_back(sig);
}

struct signal *signal_get(const char *name)
{
    for (auto i : signals)
    {
        if (strcmp(i->name, name) == 0)
        {
            return i;
        }
    }
    return NULL;
}

//this is NOT recommended, as it loops through all sigs
struct signal *signal_raise(const char *name, int64_t v, const char *fallback_msg = NULL)
{
    auto sig = signal_get(name);
    return sig;
}

//call this when possible!
struct signal *signal_raise(struct signal *sig, int64_t v, const char *fallback_msg = NULL)
{
    if (sig == NULL)
    {
        return NULL;
    }
    if (sig->value != v)
    {
        sig->resolved = SIGNAL_RAISED;
        sig->value = v;
        if (fallback_msg != NULL) //save memory
        {
            sig->fallback_msg = fallback_msg;
        }
        DEBUG("SIGNAL", (String("Raised ") + sig->name + " = " + sig->value).c_str());
    }
}

void signal_resolve(struct signal *sig, int64_t v)
{
    if (sig->resolved != SIGNAL_RESOLVED)
    {
        sig->resolved = SIGNAL_RESOLVED;
        sig->value = v;
        sig->fallback_msg = NULL; //release? still don't quite understand NULL though.
        DEBUG("SIGNAL", (String("Resolve ") + sig->name).c_str());
    }
}

void signal_resolve(struct signal *sig)
{
    signal_resolve(sig, sig->value); //default value
}

void signal_flush_store()
{
    _signal_store.begin("signal_store", false);
    _signal_store.clear();
    DEBUG("SIGNAL", "Store Flushed");
    _signal_store.end();
    for (auto i : signals)
    {
        if (i->presist_behavior == SIGNAL_PRESIST_POWERLOSS)
        {
            i->_saved_value = 0; //reset stuff
        }
    }
    signal_resolve(&SIG_FLUSH_SIGS, 0);
}

void signal_presist_init()
{
    //this loads stuff if previously saved
    for (auto i : signals)
    {
        if (i->presist_behavior == SIGNAL_PRESIST_POWERLOSS)
        {
            _signal_store.begin("signal_store", false);
            i->value = _signal_store.getLong64(i->name, 0);
            i->resolved = _signal_store.getInt((String("resolved_") + i->name).c_str(), 0);
            i->_saved_value = i->value;
            DEBUG("SIGNAL", (String("Load ") + i->name + " = " + i->value).c_str());
        }
    }
    _signal_store.end();

    if (SIG_FLUSH_SIGS.value > 0)
    {
        signal_flush_store();
    }
}

//put this at the VERY END of your runtime loop
//ensures correct presist_behavior
void signal_presist_update()
{
    for (auto i : signals)
    {
        if (i->presist_behavior == SIGNAL_PRESIST_ONCE)
        {
            signal_resolve(i);
        }
        else if (i->presist_behavior == SIGNAL_PRESIST_ONCE_AUTO_ZERO)
        {
            signal_resolve(i, 0);
        }
        else if (i->presist_behavior == SIGNAL_PRESIST_POWERLOSS && (i->value != i->_saved_value))
        {
            _signal_store.begin("signal_store", false); //preferences.h already checks _started for us
            _signal_store.putLong64(i->name, i->value);
            _signal_store.putInt((String("resolved_") + i->name).c_str(), i->resolved);
            i->_saved_value = i->value;
            DEBUG("SIGNAL", (String("Save ") + i->name + " = " + i->value).c_str());
        }
    }
    _signal_store.end();
}

//always stored value with some versatility
//i.e capable of storing strings and so on
Preferences _config_store;
typedef struct config
{
    const char *name;
    int64_t value64;
    String valueString;

    int64_t old_value64;
    String old_valueString;

    int changed;
};

std::list<struct config *> configs;

void config_register(struct config *conf)
{
    configs.push_back(conf);
}

struct config *config_get(const char *name)
{
    for (auto i : configs)
    {
        if (strcmp(i->name, name) == 0)
        {
            return i;
        }
    }
    return NULL;
}

void config_presist_init()
{
    //this loads stuff if previously saved
    for (auto i : configs)
    {
        _signal_store.begin("config_store", false);
        i->value = _signal_store.getLong64(i->name, 0);
        i->resolved = _signal_store.getInt((String("resolved_") + i->name).c_str(), 0);
        i->_saved_value = i->value;
        DEBUG("SIGNAL", (String("Load ") + i->name + " = " + i->value).c_str());
    }
    _signal_store.end();

    if (SIG_FLUSH_SIGS.value > 0)
    {
        signal_flush_store();
    }
}

//this is very stupid right now
void config_presist_update()
{
    for (auto i : configs)
    {
        if (i->value64 != i->old_value64 ||
            !i->valueString.equals(i->old_valueString))
        {
            //changed
            _config_store.begin("config_store", false); //preferences.h already checks _started for us
            _config_store.putLong64(String(i->name) + "_64", i->value64);
            _config_store.putString(String(i->name) + "_str", i->valueString);
            i->old_value64 = i->value64;
            i->old_valueString = String(i->valueString);
            i->changed = 1;
            DEBUG("CONFIG", (String("Save ") + i->name + " = " + i->value).c_str());
            signal_raise(&SIG_CONFIG_CHANGED, 1, "Configuration Changed");
        }
    }
    _config_store.end();
    return NULL;
}

void base_subsys_init()
{
    signal_register(&SIG_FLUSH_SIGS);
    signal_register(&SIG_FLUSH_CONFIG);
    signal_register(&SIG_CONFIG_CHANGED);
    signal_presist_init();
}

void base_subsys_loop()
{
    signal_presist_update();
}

#endif