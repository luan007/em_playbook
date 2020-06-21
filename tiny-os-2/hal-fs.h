#ifndef _GUARD_FS_H
#define _GUARD_FS_H

#include "FS.h"
#include "SPIFFS.h"
#include "FFat.h"
#include "defs.h"
#define USE_FS FFat

#define FS_MSG_NEW_DISK 1
#define FS_MSG_CORRUPT 2

SIGNAL(FORMAT_FS, "Format Disk", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_POWERLOSS, 0)
SIGNAL(FS_MSG, "FS Runtime Alert", SIGNAL_VIZ_ALL, SIGNAL_PRESIST_RUNTIME, 0)

void hal_fs_sig_register()
{
    signal_register(&SIG_FORMAT_FS);
}

void hal_fs_setup()
{
    if (SIG_FORMAT_FS.value > 0)
    {
        DEBUG("FS", "Formating Disk");
        USE_FS.format();
        signal_raise(&SIG_FORMAT_FS, 0);
        signal_raise(&SIG_FS_MSG, FS_MSG_NEW_DISK);
        DEBUG("FS", "Disk Formatted");
    }
    if (!USE_FS.begin())
    {
        USE_FS.format();
        signal_raise(&SIG_FS_MSG, FS_MSG_CORRUPT);
        USE_FS.begin();
    }
}

#endif