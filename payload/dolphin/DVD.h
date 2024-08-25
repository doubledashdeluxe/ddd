#pragma once

#include <common/Types.h>

typedef struct DVDCommandBlock DVDCommandBlock;

typedef void (*DVDCBCallback)(s32 result, DVDCommandBlock *block);

struct DVDCommandBlock {
    u8 _00[0x08 - 0x00];
    u32 command;
    u32 state;
    u32 offset;
    u32 length;
    void *addr;
    u8 _1c[0x1c - 0x18];
    u32 transferredSize;
    u8 _24[0x28 - 0x24];
    DVDCBCallback callback;
    u8 _2c[0x30 - 0x2c];
};
size_assert(DVDCommandBlock, 0x30);

typedef struct DVDFileInfo {
    DVDCommandBlock block;
    u8 _30[0x34 - 0x30];
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

s32 DVDGetDriveStatus(void);
