#ifndef _GUARD_H_DEF
#define _GUARD_H_DEF

#include <list>
#include <Preferences.h>

#define DBG_LEVEL_LOG 0

void DEBUG(const char *module, const char *stuff, int LEVEL = DBG_LEVEL_LOG)
{
    auto msg = String("[DBG] ") + module + " : " + stuff;
    Serial.println(msg.c_str());
}

Preferences _signal_store;

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

#define SIGNAL_RAISED 1
#define SIGNAL_RESOLVED 0

#define SIGNAL_PRESIST_ONCE 1
#define SIGNAL_PRESIST_RUNTIME 0
#define SIGNAL_PRESIST_POWERLOSS 2

typedef struct signal
{
    const char *name;
    const char *fallback_msg;
    int visibility;
    int presist_behavior;
    int resolved;
    int value;
    int _saved_value; //to ensure not writing too many times during loop
};

#define SIGNAL(NAME, default_msg, visibility, presist_behavior, default_value) struct signal NAME = {#NAME, default_msg, visibility, presist_behavior, 0, default_value};

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
struct signal *signal_raise(const char *name, int v, const char *fallback_msg)
{
    auto sig = signal_get(name);
    return sig;
}

//call this when possible!
struct signal *signal_raise(struct signal *sig, int v, const char *fallback_msg)
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
    }
}

void signal_resolve(struct signal *sig)
{
    sig->resolved = SIGNAL_RESOLVED;
}

void signal_presist_init()
{
    //this loads stuff if previously saved
    for (auto i : signals)
    {
        if (i->presist_behavior == SIGNAL_PRESIST_POWERLOSS)
        {
            _signal_store.begin("signal_store", false);
            i->value = _signal_store.getInt(i->name, 0);
            i->resolved = _signal_store.getInt((String("resolved_") + i->name).c_str(), 0);
            i->_saved_value = i->value;
            DEBUG("SIGNAL", (String("Load ") + i->name + " = " + i->value).c_str());
        }
    }
    _signal_store.end();
}

//put this at the VERY END of your runtime loop
//ensures correct presist_behavior
void signal_presist_update()
{
    bool _store_inited = false;
    for (auto i : signals)
    {
        if (i->presist_behavior == SIGNAL_PRESIST_ONCE)
        {
            signal_resolve(i);
        }
        else if (i->presist_behavior == SIGNAL_PRESIST_POWERLOSS && (i->value != i->_saved_value))
        {
            _signal_store.begin("signal_store", false); //preferences.h already checks _started for us
            _signal_store.putInt(i->name, i->value);
            _signal_store.putInt((String("resolved_") + i->name).c_str(), i->resolved);
            i->_saved_value = i->value;
            DEBUG("SIGNAL", (String("Save ") + i->name + " = " + i->value).c_str());
        }
    }
    _signal_store.end();
}

#endif