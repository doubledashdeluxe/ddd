#pragma once

#include <cube/storage/Storage.hh>
extern "C" {
#include <dolphin/CARD.h>
}
#include <portable/Array.hh>
#include <portable/UniquePtr.hh>

class VirtualCard : public Storage::Observer {
public:
    s32 freeBlocks(s32 *bytesNotUsed, s32 *filesNotUsed);
    s32 check();

    s32 probeEx(s32 *memSize, s32 *sectorSize);

    s32 mount(void *workArea, CARDCallback detachCallback);
    s32 unmount();

    s32 format();

    s32 fastOpen(s32 fileNo, CARDFileInfo *fileInfo);
    s32 open(const char *fileName, CARDFileInfo *fileInfo);
    s32 close(CARDFileInfo *fileInfo);

    s32 create(const char *fileName, u32 size, CARDFileInfo *fileInfo);

    s32 read(CARDFileInfo *fileInfo, void *addr, s32 length, s32 offset);
    s32 write(CARDFileInfo *fileInfo, const void *addr, s32 length, s32 offset);

    s32 fastRemove(s32 fileNo);
    s32 remove(const char *fileName);

    s32 getStatus(s32 fileNo, CARDStat *stat);
    s32 setStatus(s32 fileNo, CARDStat *stat);

    s32 rename(const char *oldName, const char *newName);

    static void Init();
    static VirtualCard *Instance(s32 chan);

private:
    enum {
        SectorSize = 0x2000,
        FileHeaderSize = 0x40,
    };

    struct File {
        bool isValid;
        Array<char, 256> path;
        CARDStat stat;
    };

    VirtualCard(s32 chan);

    void onAdd(const char *prefix) override;
    void onRemove(const char *prefix) override;

    Array<char, 256> getPath(CARDStat &stat);

    static bool ReadStat(Storage::FileHandle &file, CARDStat &stat);
    static bool WriteStat(Storage::FileHandle &file, CARDStat &stat);

    s32 m_chan;
    bool m_isMounted;
    Array<char, 256> m_dirPath;
    Array<File, 127> m_files;

    static Array<UniquePtr<VirtualCard>, CARD_NUM_CHANS> s_instances;
};
