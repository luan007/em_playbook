#ifndef _GUARD_H_DEF
#define _GUARD_H_DEF

#include <list>

struct schedule
{
    long next_wake; //current time = 0, computed = internal delta + current
    void (*compute)(int);
    const char* name;
};

std::list<struct schedule *> schedules;
void schedules_push(struct schedule *sch)
{
    schedules.push_back(sch);
}

#endif