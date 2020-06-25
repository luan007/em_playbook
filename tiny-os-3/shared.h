#ifndef _GUARD_H_DEF
#define _GUARD_H_DEF
//TODO: note, wear levelling should be considered
#include <list>
#include <Preferences.h>
#include "esp_task_wdt.h"

//////////////DEBUG

#define DBG_LEVEL_LOG 0
void DEBUG(const char *module, const char *stuff, int LEVEL = DBG_LEVEL_LOG)
{
    if (LEVEL < 0)
        return;
    auto msg = String("[DBG] ") + module + " : " + stuff;
    Serial.println(msg.c_str());
}

//////////////WDT FOR BAD STUFF WHEN NEEDED
#define CPU_WDT 1

//TODO: see if this actually works
void wdt_feed(void *pvParameters)
{
    (void)pvParameters;
    while (1)
    {
        esp_task_wdt_reset();
        vTaskDelay(1);
    }
}

void wdt_clear()
{
    esp_task_wdt_reset();
}

#define WDT_TIMEOUT_SEC 2
void wdt_start()
{
    TaskHandle_t handle;
    //enable interrupt
    xTaskCreatePinnedToCore(
        wdt_feed, "wdt_feed",
        4096, NULL, 2, &handle, CPU_WDT);

    const char *result;

    esp_task_wdt_init(WDT_TIMEOUT_SEC, true);
    esp_err_t err = esp_task_wdt_add(handle);
    switch (err)
    {
    case ESP_OK:
        result = "OK";
        break;
    case ESP_ERR_INVALID_ARG:
        result = "ERR = INVALID ARG";
        break;
    case ESP_ERR_NO_MEM:
        result = "ERR = NO MEM";
        break;
    case ESP_ERR_INVALID_STATE:
        result = "ERR = INVALID STATE";
        break;
    default:
        result = "ERR = OTHER";
        break;
    }
    DEBUG("WDT", (String("RESULT = ") + result).c_str());
}

extern void sig_external_event(); //this should be defined somewhere

//////////////SCHEDULER

std::list<uint32_t (*)()> schedulers;
uint32_t schedule_compute_millis()
{
    uint32_t next_wake_in_millis = (1000 * 60 * 60 * 6); //6 hour no matter what?
    for (auto scheduler : schedulers)
    {
        auto purposed = scheduler();
        DEBUG("SCHEDULER", (String("WAKE PURPOSAL = ") + purposed).c_str());
        if (purposed == 0)
            continue;
        next_wake_in_millis = next_wake_in_millis < purposed ? next_wake_in_millis : purposed;
    }
    DEBUG("SCHEDULER", (String("WAKE POINT = ") + next_wake_in_millis).c_str());
    return next_wake_in_millis;
}

//////////////SIGNAL

//this will trigger update immediately
#define SIG_IMMEDIATE (1 << 7)
//this will popup to app
#define SIG_APP (1 << 2)
//this will popup to os-level app (not used)
#define SIG_OS (1 << 1)
#define SIG_NONE 0
#define SIG_ALL 0xFFFF

#define SIG_ONCE_ZERO 1
#define SIG_ONCE 1
#define SIG_RUNTIME 0
#define SIG_POWERLOSS 2

Preferences _signal_store;
typedef struct signal
{
    const char *name;
    int visibility;
    int presist_behavior;
    int value;
    int debug_level;
    int triggered;
    int _saved_value; //to ensure not writing too many times during loop
    int _notified_value;
};

#define SIGNAL(NAME, visibility, presist_behavior, default_value) struct signal SIG_##NAME = {#NAME, visibility, presist_behavior, default_value, 0, 0};

SIGNAL(CONFIG_CHANGED, SIG_ALL, SIG_RUNTIME, 0)
SIGNAL(WAKE, SIG_ALL, SIG_RUNTIME, 0)
SIGNAL(NOTIFY_RELEASE, SIG_ALL, SIG_IMMEDIATE, 0)
SIGNAL(BEFORE_SLEEP, SIG_ALL, SIG_IMMEDIATE, 0)
SIGNAL(WAKE_AFTER, SIG_ALL, SIG_RUNTIME, 0)
SIGNAL(SYS_BROKE, SIG_ALL, SIG_IMMEDIATE, 0)
SIGNAL(SYS_MSG, SIG_ALL, SIG_IMMEDIATE, 0)
SIGNAL(ALLOW_LOOP, SIG_NONE, SIG_RUNTIME, 0)
SIGNAL(NO_MORE_OP, SIG_ALL, SIG_IMMEDIATE, 0)
SIGNAL(DBG_MODE, SIG_ALL, SIG_POWERLOSS, 0)

std::list<struct signal *> signals;

void sig_reg(struct signal *sig)
{
    signals.push_back(sig);
}

void sig_external_trigger()
{
    bool emit = false;
    for (auto sig : signals)
    {
        if ((sig->visibility & SIG_APP ||
             sig->visibility & SIG_IMMEDIATE) &&
            sig->triggered &&
            sig->_notified_value != sig->value)
        {
            sig->_notified_value = sig->value;
            emit = true;
        }
    }
    if (emit)
    {
        sig_external_event();
    }
}

struct signal *sig_find(const char *name)
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

struct signal *sig_set(struct signal *sig, int v)
{
    if (sig == NULL)
    {
        return NULL;
    }
    if (sig->value != v)
    {
        sig->triggered = 1;
        sig->value = v;
        DEBUG("SIG", (String(sig->name) + " = " + sig->value).c_str(), sig->debug_level);
        if (sig->visibility & SIG_IMMEDIATE)
        {
            sig_external_trigger();
        }
    }
}

struct signal *sig_set(const char *name, int v)
{
    auto sig = sig_find(name);
    return sig_set(sig, v);
}

void sig_clear(struct signal *sig, int v)
{
    if (sig == NULL)
        return;
    if (sig->triggered)
    {
        sig->triggered = 0;
        DEBUG("SIG", (String("[RESOLVE] ") + sig->name).c_str(), sig->debug_level);
    }
    sig->value = v;
}

void sig_clear(struct signal *sig)
{
    sig_clear(sig, sig->value); //default value
}

void sig_clear(const char *name, int v)
{
    auto sig = sig_find(name);
    sig_clear(sig, v);
}

void sig_clear(const char *name)
{
    auto sig = sig_find(name);
    sig_clear(sig);
}

void sig_flush_store()
{
    _signal_store.begin("signal_store", false);
    _signal_store.clear();
    DEBUG("SIG", "Store Flushed");
    _signal_store.end();
    for (auto i : signals)
    {
        if (i->presist_behavior == SIG_POWERLOSS)
        {
            i->_saved_value = 0; //reset stuff
        }
    }
}

void sig_init()
{
    for (auto i : signals)
    {
        if (i->presist_behavior == SIG_POWERLOSS)
        {
            _signal_store.begin("signal_store", false);
            i->value = _signal_store.getInt(i->name);
            i->triggered = _signal_store.getInt((String("t") + i->name).c_str(), 0);
            i->_saved_value = i->value;
            i->_notified_value = i->value;
            DEBUG("SIG", (String("Load ") + i->name + " = " + i->value).c_str());
        }
    }
    _signal_store.end();
}

void sig_save(bool LAST_CYCLE = false)
{
    static unsigned long sig_last_save = 0;
    static unsigned long sig_save_min_freq = 10000; //limit qps (like 10)
    //qps limit
    bool can_save = LAST_CYCLE || (millis() - sig_last_save > sig_save_min_freq);
    bool has_saved = false;
    if (!can_save)
    {
        return;
    }

    for (auto i : signals)
    {
        if (i->presist_behavior == SIG_POWERLOSS && (i->value != i->_saved_value))
        {
            has_saved = true;
            _signal_store.begin("signal_store", false); //preferences.h already checks _started for us
            _signal_store.putInt(i->name, i->value);
            _signal_store.putInt((String("t") + i->name).c_str(), i->triggered);
            i->_saved_value = i->value;
            DEBUG(">>> WRITE", (String(i->name) + " = " + i->value).c_str());
        }
    }
    if (has_saved)
    {
        sig_last_save = millis();
        DEBUG("SIG-SAVE", (String("Force ") + " = " + LAST_CYCLE).c_str());
    }
    _signal_store.end();
}

void sig_tick()
{
    for (auto sig : signals)
    {
        if (sig->visibility == SIG_ONCE_ZERO && sig->_notified_value == sig->value)
        {
            sig->triggered = 0;
            sig->value = 0;
        }
        else if (sig->visibility == SIG_ONCE && sig->_notified_value == sig->value)
        {
            sig->triggered = 0;
        }
    }
    sig_save();
    sig_external_trigger();
}

////////////// CONFIG

Preferences _config_store;
typedef struct config
{
    const char *name;
    int value64;
    String valueString;

    int old_value64;
    String old_valueString;

    int changed;
};

#define CONFIG(NAME, default_64, str) struct config CFG_##NAME = {#NAME, default_64, String(str)};

std::list<struct config *> configs;

void cfg_reg(struct config *conf)
{
    configs.push_back(conf);
}

void cfg_flush_store()
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
}

struct config *cfg_find(const char *name)
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

void cfg_init()
{
    int has_change = 0;
    //this loads stuff if previously saved
    for (auto i : configs)
    {
        _config_store.begin("config_store", false);
        i->value64 = _config_store.getInt((String(i->name) + "i").c_str(), i->value64);
        i->valueString = _config_store.getString((String(i->name) + "s").c_str(), i->valueString);
        i->old_value64 = _config_store.getInt((String(i->name) + "i").c_str());
        i->old_valueString = _config_store.getString((String(i->name) + "s").c_str());
        DEBUG("CONFIG", (String("Load ") + i->name + " V64 = " + i->value64).c_str());
        DEBUG("CONFIG", (String("Load ") + i->name + " STR = " + i->valueString).c_str());
        has_change = 1;
    }
    _config_store.end();
    if (has_change)
    {
        sig_set(&SIG_CONFIG_CHANGED, 1);
    }
}

void cfg_clear_change(struct config *i)
{
    i->changed = 0;
}

int cfg_autosave(struct config *i, bool auto_end = true)
{
    if (i->value64 != i->old_value64 ||
        !i->valueString.equals(i->old_valueString))
    {
        //changed
        _config_store.begin("config_store", false); //preferences.h already checks _started for us
        _config_store.putInt((String(i->name) + "i").c_str(), i->value64);
        _config_store.putString((String(i->name) + "s").c_str(), i->valueString);
        if (auto_end)
        {
            _config_store.end();
        }
        i->old_value64 = i->value64;
        i->old_valueString = String(i->valueString);
        i->changed = 1;
        DEBUG("CONFIG", (String("Save ") + i->name + " V64 = " + i->value64).c_str());
        DEBUG("CONFIG", (String("Save ") + i->name + " STR = " + i->valueString).c_str());
        //ROM
        sig_set(&SIG_CONFIG_CHANGED, 1);

        return 1;
    }
    else
    {
        i->changed = 0; //TODO: this might be wrong
    }
    return 0;
}

int cfg_set(struct config *i, String val)
{
    i->valueString.clear();
    i->valueString = val;
    return cfg_autosave(i);
}

int cfg_set(struct config *i, const char *val)
{
    return cfg_set(i, String(val));
}

int cfg_set(struct config *i, int val)
{
    i->value64 = val;
    return cfg_autosave(i);
}

void cfg_tick()
{
    for (auto i : configs)
    {
        cfg_autosave(i, false);
    }
    _config_store.end();
}

#endif