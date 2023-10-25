#include "DVD.h"

typedef struct {
    u8 isDir : 8;
    u32 stringOffset : 24;
    union {
        struct {
            u32 parent;
            u32 next;
        } dir;
        struct {
            u32 startAddr;
            u32 length;
        } file;
    };
} FstEntry;
static_assert(sizeof(FstEntry) == 0xc);

extern char *FstStringStart;
extern FstEntry *FstStart;
extern u32 MaxEntryNum;

BOOL DVDOpenDir(const char *dirName, DVDDir *dir) {
    s32 entrynum = DVDConvertPathToEntrynum(dirName);
    return DVDFastOpenDir(entrynum, dir);
}

BOOL DVDFastOpenDir(s32 entrynum, DVDDir *dir) {
    if (entrynum < 0) {
        return FALSE;
    }

    if ((u32)entrynum >= MaxEntryNum) {
        return FALSE;
    }

    if (!FstStart[entrynum].isDir) {
        return FALSE;
    }

    dir->entrynum = entrynum;
    dir->location = entrynum + 1;
    dir->next = FstStart[entrynum].dir.next;
    return TRUE;
}

BOOL DVDReadDir(DVDDir *dir, DVDDirEntry *dirent) {
    if (dir->location <= dir->entrynum) {
        return FALSE;
    }

    if (dir->location >= dir->next) {
        return FALSE;
    }

    dirent->entrynum = dir->location;
    dirent->isDir = !!FstStart[dir->location].isDir;
    dirent->name = FstStringStart + FstStart[dir->location].stringOffset;

    if (FstStart[dir->location].isDir) {
        dir->location = FstStart[dir->location].dir.next;
    } else {
        dir->location++;
    }

    return TRUE;
}

BOOL DVDCloseDir(DVDDir * /* dir */) {
    return TRUE;
}
