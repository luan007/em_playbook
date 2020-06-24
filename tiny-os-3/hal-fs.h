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

void _dbg_ls_dir(const char *c, int levels)
{
    File root = USE_FS.open(c);
    if (!root)
    {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
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
                _dbg_ls_dir(file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
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