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

SIGNAL(FS_FREE, SIG_NONE, SIG_RUNTIME, 0)
SIGNAL(FS_USED_COMPUTED, SIG_NONE, SIG_RUNTIME, 0)
SIGNAL(FS_TOTAL, SIG_NONE, SIG_RUNTIME, 0)

void update_fs_info()
{
    sig_set(&SIG_FS_FREE, USE_FS.freeBytes());
    sig_set(&SIG_FS_TOTAL, USE_FS.totalBytes());
    Serial.printf("Disk Status: %d : %d / %d", SIG_FS_FREE.value, SIG_FS_USED_COMPUTED.value, SIG_FS_TOTAL.value);
}

int _dbg_ls_dir(const char *c, int levels)
{
    int sz = 0;
    File root = USE_FS.open(c);
    if (!root)
    {
        Serial.println("- failed to open directory");
        return 0;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return 0;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                sz += _dbg_ls_dir(file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
            sz += file.size();
        }
        file = root.openNextFile();
    }
    if (strcmp(c, "/") == 0)
    {
        sig_set(&SIG_FS_USED_COMPUTED, sz);
        update_fs_info();
    }
    return sz;
}

void _rm_recur(const char *c)
{
    Serial.print("RM - ");
    Serial.println(c);
    File root = USE_FS.open(c);
    if (!root.isDirectory())
    {
        root.close();
        USE_FS.remove(c);
        return;
    }
    File file = root.openNextFile();
    while (file)
    {
        _rm_recur(file.name());
        file = root.openNextFile();
    }
    USE_FS.rmdir(c);
}

void hal_fs_wipe()
{
    USE_FS.end();
    USE_FS.format();
}

void hal_fs_setup()
{
    if (!USE_FS.begin())
    {
        USE_FS.format();
        sig_set(&SIG_FS_MSG, FS_MSG_CORRUPT);
        USE_FS.begin();
    }
    _dbg_ls_dir("/", 1000);
}

#endif