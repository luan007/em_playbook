#ifndef _GUARD_H_DEF
#define _GUARD_H_DEF
//TODO: note, wear levelling should be considered
#include "time.h"
#include <sys/time.h>
#include <list>
#include <Preferences.h>

#define DBG_LEVEL_LOG 0

void DEBUG(const char *module, const char *stuff, int LEVEL = DBG_LEVEL_LOG)
{
    if (LEVEL < 0)
        return;
    auto msg = String("[DBG] ") + module + " : " + stuff;
    Serial.println(msg.c_str());
}

int64_t _rtc_mil_offset = 0;

int64_t epochs()
{
    int64_t milliseconds = _rtc_mil_offset + (int64_t)millis();
    return milliseconds;
}

#define r_millis epochs

long r_secs() //trustworthy time!
{
    return static_cast<long>(time(NULL));
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

#define WAKE_REASON_ULP 2
#define WAKE_REASON_NONE 1
#define WAKE_REASON_TIMER 3

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
    int resolved; //should be raised
    int value;
    int _saved_value; //to ensure not writing too many times during loop
    int debug_level;
    int reported_value;
};

#define SIGNAL(NAME, default_msg, visibility, presist_behavior, default_value) struct signal SIG_##NAME = {#NAME, default_msg, visibility, presist_behavior, 0, default_value};

SIGNAL(SYS_BROKE, "Broken system", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, 0)

SIGNAL(FLUSH_SIGS, "Flush Store", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, 0)
SIGNAL(FLUSH_CONFIG, "Flush Config", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, 0)
SIGNAL(CONFIG_CHANGED, "Config Changed", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_ONCE_AUTO_ZERO, 0)
SIGNAL(NO_SLEEP, "When this is on, the system cannot goto sleep (during update or network activity)", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)
SIGNAL(BEFORE_SLEEP, "This will fire before sleep", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)
SIGNAL(NEXT_WAKE, "Signal for saving next wake", SIGNAL_VIZ_NONE, SIGNAL_PRESIST_RUNTIME, 40 * 1000) //min wake time 5000ms
SIGNAL(NEXT_SLEEP, "Compute nearest sleep timeslot", SIGNAL_VIZ_NONE, SIGNAL_PRESIST_RUNTIME, 10)    //min sleep right after 10ms
SIGNAL(WAKE_REASON, "Wake reason", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)
SIGNAL(TIME_VALID, "time valid from powerloss", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, 0)
SIGNAL(FAC_RESET, "wipe everything", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_ONCE_AUTO_ZERO, 0)

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

//call this when possible!
struct signal *signal_raise(struct signal *sig, int v, const char *fallback_msg = NULL)
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
        DEBUG("SIGNAL", (String("Raised ") + sig->name + " = " + sig->value).c_str(), sig->debug_level);
    }
}

//this is NOT recommended, as it loops through all sigs
struct signal *signal_raise(const char *name, int v, const char *fallback_msg = NULL)
{
    auto sig = signal_get(name);
    return signal_raise(sig, v, fallback_msg);
}

void signal_resolve(struct signal *sig, int v)
{
    if (sig->resolved != SIGNAL_RESOLVED)
    {
        sig->resolved = SIGNAL_RESOLVED;
        sig->value = v;
        sig->fallback_msg = NULL; //release? still don't quite understand NULL though.
        DEBUG("SIGNAL", (String("Resolve ") + sig->name).c_str(), sig->debug_level);
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
            i->value = _signal_store.getInt(i->name);
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
unsigned long sig_last_save = 0;
unsigned long sig_save_min_freq = 10000; //limit qps (like 10)
void signal_presist_update(bool LAST_CYCLE = false)
{
    //qps limit
    bool can_save = LAST_CYCLE || (millis() - sig_last_save > sig_save_min_freq);
    bool has_saved = false;
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
            if (can_save)
            {
                has_saved = true;
                _signal_store.begin("signal_store", false); //preferences.h already checks _started for us
                _signal_store.putInt(i->name, i->value);
                _signal_store.putInt((String("resolved_") + i->name).c_str(), i->resolved);
                i->_saved_value = i->value;
                DEBUG("SIGNAL", (String("Save ") + i->name + " = " + i->value).c_str());
            }
        }
    }
    if (has_saved)
    {
        sig_last_save = millis();
    }
    _signal_store.end();
}

//always stored value with some versatility
//i.e capable of storing strings and so on
Preferences _config_store;
typedef struct config
{
    const char *name;
    int value64;
    String valueString;

    String meta;

    int old_value64;
    String old_valueString;

    int changed;
};

#define CONFIG(NAME, meta, default_64, str) struct config CFG_##NAME = {#NAME, default_64, String(str), String(meta)};

std::list<struct config *> configs;

void config_register(struct config *conf)
{
    configs.push_back(conf);
}

void config_flush_store()
{
    _config_store.begin("config_store", false);
    _config_store.clear();
    DEBUG("CONFIG", "Store Flushed");
    _config_store.end();
    for (auto i : configs)
    {
        i->old_valueString = String();
        i->old_value64 = 0;
    }
    signal_resolve(&SIG_FLUSH_CONFIG, 0);
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
    int has_change = 0;
    //this loads stuff if previously saved
    for (auto i : configs)
    {
        _config_store.begin("config_store", false);
        if (SIG_FLUSH_CONFIG.value <= 0)
        {
            i->value64 = _config_store.getInt((String(i->name) + "i").c_str(), i->value64);
            i->valueString = _config_store.getString((String(i->name) + "s").c_str(), i->valueString);
            i->old_value64 = _config_store.getInt((String(i->name) + "i").c_str());
            i->old_valueString = _config_store.getString((String(i->name) + "s").c_str());
        }
        i->changed = 1;

        DEBUG("CONFIG", (String("Load ") + i->name + " V64 = " + i->value64).c_str());
        DEBUG("CONFIG", (String("Load ") + i->name + " STR = " + i->valueString).c_str());
        has_change = 1;
    }
    _config_store.end();

    if (SIG_FLUSH_CONFIG.value > 0)
    {
        config_flush_store();
    }

    if (has_change)
    {
        signal_raise(&SIG_CONFIG_CHANGED, 1, "Configuration Changed");
    }
}

//this is very stupid right now
void config_presist_update()
{
    int has_change = 0;
    for (auto i : configs)
    {
        if (i->value64 != i->old_value64 ||
            !i->valueString.equals(i->old_valueString))
        {
            //changed
            _config_store.begin("config_store", false); //preferences.h already checks _started for us
            _config_store.putInt((String(i->name) + "i").c_str(), i->value64);
            _config_store.putString((String(i->name) + "s").c_str(), i->valueString);
            i->old_value64 = i->value64;
            i->old_valueString = String(i->valueString);
            i->changed = 1;
            has_change = 1;
            DEBUG("CONFIG", (String("Save ") + i->name + " V64 = " + i->value64).c_str());
            DEBUG("CONFIG", (String("Save ") + i->name + " STR = " + i->valueString).c_str());
            //ROM
        }
        else
        {
            i->changed = 0; //restore changed value in RAM
        }
    }
    if (has_change > 0)
    {
        signal_raise(&SIG_CONFIG_CHANGED, 1, "Configuration Changed");
    }
    _config_store.end();
}

void base_subsys_init()
{
    signal_register(&SIG_FAC_RESET);
    signal_register(&SIG_NO_SLEEP);
    signal_register(&SIG_WAKE_REASON);
    signal_register(&SIG_NEXT_WAKE);
    signal_register(&SIG_FLUSH_SIGS);
    signal_register(&SIG_FLUSH_CONFIG);
    signal_register(&SIG_TIME_VALID);
    signal_register(&SIG_CONFIG_CHANGED);
    signal_register(&SIG_BEFORE_SLEEP);
    signal_register(&SIG_SYS_BROKE);
    signal_presist_init();
    config_presist_init();
}

void base_subsys_loop()
{
    signal_presist_update(SIG_BEFORE_SLEEP.value > 0);
    config_presist_update();
}

//note, this will write stuff into nvs & resolve stuff
void base_force_tick()
{
    signal_presist_update(true);
    config_presist_update();
    DEBUG("BASE", "Force tick once");
}

#endif