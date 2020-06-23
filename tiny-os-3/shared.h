#ifndef _GUARD_H_DEF
#define _GUARD_H_DEF
//TODO: note, wear levelling should be considered
#include "time.h"
#include <sys/time.h>
#include <list>
#include <Preferences.h>

//////////////DEBUG

#define DBG_LEVEL_LOG 0
void DEBUG(const char *module, const char *stuff, int LEVEL = DBG_LEVEL_LOG)
{
    if (LEVEL < 0)
        return;
    auto msg = String("[DBG] ") + module + " : " + stuff;
    Serial.println(msg.c_str());
}

//////////////SCHEDULER

std::list<uint32_t (*)()> schedulers;
uint32_t schedule_compute_millis()
{
    uint32_t next_wake_in_millis = (1000 * 60 * 60 * 6); //6 hour no matter what?
    for (auto scheduler : schedulers)
    {
        auto purposed = scheduler();
        next_wake_in_millis = next_wake_in_millis < purposed ? next_wake_in_millis : purposed;
    }
    return next_wake_in_millis;
}


//////////////SIGNAL

//this will trigger update immediately
#define SIGNAL_IMMEDIATE (1 << 7)
//this will popup to app
#define SIGNAL_APP (1 << 2)
//this will popup to os-level app (not used)
#define SIGNAL_OS (1 << 1)
#define SIGNAL_NONE 0
#define SIGNAL_ALL 0xFFFF

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

std::list<struct signal *> signals;

void sig_reg(struct signal *sig)
{
    signals.push_back(sig);
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
        sig->value = v;
        DEBUG("SIGNAL", (String("Clear ") + sig->name).c_str(), sig->debug_level);
    }
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
    DEBUG("SIGNAL", "Store Flushed");
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
            DEBUG("SIGNAL", (String("Load ") + i->name + " = " + i->value).c_str());
        }
    }
    _signal_store.end();
}

unsigned long sig_last_save = 0;
unsigned long sig_save_min_freq = 10000; //limit qps (like 10)
void sig_save(bool LAST_CYCLE = false)
{
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
            DEBUG("SIGNAL", (String("Save ") + i->name + " = " + i->value).c_str());
        }
    }
    if (has_saved)
    {
        sig_last_save = millis();
    }
    _signal_store.end();
}

void sig_tick()
{
    for (auto sig : signals)
    {
        if (sig->visibility & SIG_ONCE)
        {
            sig->triggered = 0;
        }
    }
    sig_save();
}

//////////////SYS

void sys_init()
{
    sig_init();
}

void sys_tick()
{
    sig_tick();
}

#endif