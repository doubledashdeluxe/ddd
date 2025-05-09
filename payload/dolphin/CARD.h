#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

enum {
    CARD_FILENAME_MAX = 32,
};

enum {
    CARD_NUM_CHANS = 2,
};

enum {
    CARD_RESULT_READY = 0,
    CARD_RESULT_NOCARD = -3,
    CARD_RESULT_NOFILE = -4,
    CARD_RESULT_EXIST = -7,
    CARD_RESULT_INSSPACE = -9,
    CARD_RESULT_NAMETOOLONG = -12,
};

typedef struct CARDFileInfo {
    s32 chan;
    s32 fileNo;
    u8 _08[0x14 - 0x08];
} CARDFileInfo;
size_assert(CARDFileInfo, 0x14);

typedef struct CARDStat {
    char fileName[CARD_FILENAME_MAX];
    u32 length;
    u32 time;
    char gameName[4];
    char company[2];
    u8 bannerFormat;
    u8 _2f[0x30 - 0x2f];
    u32 iconAddr;
    u16 iconFormat;
    u16 iconSpeed;
    u32 commentAddr;
    u8 _3c[0x6c - 0x3c];
} CARDStat;
size_assert(CARDStat, 0x6c);

typedef void (*CARDCallback)(s32 chan, s32 result);

s32 REPLACED(CARDFreeBlocks)(s32 chan, s32 *bytesNotUsed, s32 *filesNotUsed);
REPLACE s32 CARDFreeBlocks(s32 chan, s32 *bytesNotUsed, s32 *filesNotUsed);
s32 REPLACED(CARDCheck)(s32 chan);
REPLACE s32 CARDCheck(s32 chan);

s32 REPLACED(CARDProbeEx)(s32 chan, s32 *memSize, s32 *sectorSize);
REPLACE s32 CARDProbeEx(s32 chan, s32 *memSize, s32 *sectorSize);

s32 REPLACED(CARDMount)(s32 chan, void *workArea, CARDCallback detachCallback);
REPLACE s32 CARDMount(s32 chan, void *workArea, CARDCallback detachCallback);
s32 REPLACED(CARDUnmount)(s32 chan);
REPLACE s32 CARDUnmount(s32 chan);

s32 REPLACED(CARDFormat)(s32 chan);
REPLACE s32 CARDFormat(s32 chan);

s32 REPLACED(CARDFastOpen)(s32 chan, s32 fileNo, CARDFileInfo *fileInfo);
REPLACE s32 CARDFastOpen(s32 chan, s32 fileNo, CARDFileInfo *fileInfo);
s32 REPLACED(CARDOpen)(s32 chan, const char *fileName, CARDFileInfo *fileInfo);
REPLACE s32 CARDOpen(s32 chan, const char *fileName, CARDFileInfo *fileInfo);
s32 REPLACED(CARDClose)(CARDFileInfo *fileInfo);
REPLACE s32 CARDClose(CARDFileInfo *fileInfo);

s32 REPLACED(CARDCreate)(s32 chan, const char *fileName, u32 size, CARDFileInfo *fileInfo);
REPLACE s32 CARDCreate(s32 chan, const char *fileName, u32 size, CARDFileInfo *fileInfo);

s32 REPLACED(CARDRead)(CARDFileInfo *fileInfo, void *addr, s32 length, s32 offset);
REPLACE s32 CARDRead(CARDFileInfo *fileInfo, void *addr, s32 length, s32 offset);
s32 REPLACED(CARDWrite)(CARDFileInfo *fileInfo, const void *addr, s32 length, s32 offset);
REPLACE s32 CARDWrite(CARDFileInfo *fileInfo, const void *addr, s32 length, s32 offset);

s32 REPLACED(CARDFastDelete)(s32 chan, s32 fileNo);
REPLACE s32 CARDFastDelete(s32 chan, s32 fileNo);
s32 REPLACED(CARDDelete)(s32 chan, const char *fileName);
REPLACE s32 CARDDelete(s32 chan, const char *fileName);

s32 REPLACED(CARDGetStatus)(s32 chan, s32 fileNo, CARDStat *stat);
REPLACE s32 CARDGetStatus(s32 chan, s32 fileNo, CARDStat *stat);
s32 REPLACED(CARDSetStatus)(s32 chan, s32 fileNo, CARDStat *stat);
REPLACE s32 CARDSetStatus(s32 chan, s32 fileNo, CARDStat *stat);

s32 REPLACED(CARDRename)(s32 chan, const char *oldName, const char *newName);
REPLACE s32 CARDRename(s32 chan, const char *oldName, const char *newName);

// Not a public function
void CARDUpdateIconOffsets(void *block, CARDStat *stat);
