#ifndef _GUARD_FS_H
#define _GUARD_FS_H

#include "FS.h"
#include "SPIFFS.h"
#include "FFat.h"
#include "shared.h"
#define USE_FS FFat

#define FS_MSG_NEW_DISK 1
#define FS_MSG_CORRUPT 2

SIGNAL(FS_MSG, SIG_ALL, SIG_RUNTIME, 0)

void hal_fs_setup()
{
    if (!USE_FS.begin())
    {
        USE_FS.format();
        sig_set(&SIG_FS_MSG, FS_MSG_CORRUPT);
        USE_FS.begin();
    }
}

#endif