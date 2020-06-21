#ifndef _GUARD_FS_H
#define _GUARD_FS_H

#include "FS.h"
#include "SPIFFS.h"
#include "FFat.h"
#define USE_FS FFat

void hal_fs_setup()
{
    // USE_FS.format();
    USE_FS.begin();
}

#endif