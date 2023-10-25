#pragma once

#include <common/Types.h>

typedef struct DVDFileInfo {
    u8 _00[0x34 - 0x00];
    u32 length;
    u8 _38[0x3c - 0x38];
} DVDFileInfo;
static_assert(sizeof(DVDFileInfo) == 0x3c);

typedef struct DVDDir {
    u32 entrynum;
    u32 location;
    u32 next;
} DVDDir;
static_assert(sizeof(DVDDir) == 0xc);

typedef struct DVDDirEntry {
    u32 entrynum;
    BOOL isDir;
    char *name;
} DVDDirEntry;
static_assert(sizeof(DVDDirEntry) == 0xc);

BOOL DVDOpen(const char *fileName, DVDFileInfo *fileInfo);
s32 DVDReadPrio(DVDFileInfo *fileInfo, void *addr, s32 length, s32 offset, s32 prio);
BOOL DVDClose(DVDFileInfo *fileInfo);

BOOL DVDOpenDir(const char *dirName, DVDDir *dir);
BOOL DVDReadDir(DVDDir *dir, DVDDirEntry *dirent);
BOOL DVDCloseDir(DVDDir *dir);
