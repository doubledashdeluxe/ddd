#pragma once

#include <common/Types.h>

typedef struct DVDFileInfo {
    u8 _00[0x34 - 0x00];
    u32 length;
    u8 _38[0x3c - 0x38];
} DVDFileInfo;
size_assert(DVDFileInfo, 0x3c);

typedef struct DVDDir {
    u32 entrynum;
    u32 location;
    u32 next;
} DVDDir;
size_assert(DVDDir, 0xc);

typedef struct DVDDirEntry {
    u32 entrynum;
    BOOL isDir;
    char *name;
} DVDDirEntry;
size_assert(DVDDirEntry, 0xc);

BOOL DVDOpen(const char *fileName, DVDFileInfo *fileInfo);
s32 DVDReadPrio(DVDFileInfo *fileInfo, void *addr, s32 length, s32 offset, s32 prio);
BOOL DVDClose(DVDFileInfo *fileInfo);

BOOL DVDFastOpenDir(s32 entrynum, DVDDir *dir);
BOOL DVDOpenDir(const char *dirName, DVDDir *dir);
BOOL DVDReadDir(DVDDir *dir, DVDDirEntry *dirent);
BOOL DVDCloseDir(DVDDir *dir);

s32 DVDConvertPathToEntrynum(const char *path);
BOOL DVDConvertEntrynumToPath(s32 entrynum, char *path, u32 maxlen);
