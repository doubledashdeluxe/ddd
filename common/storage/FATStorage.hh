#pragma once

#include "common/storage/Storage.hh"

extern "C" {
// clang-format off
#include <ff/ff.h>
#include <ff/diskio.h>
// clang-format on
}

class FATStorage : protected Storage {
protected:
    class File : public Storage::File {
    public:
        File();

    private:
        void close() override;
        bool read(void *dst, u32 size, u32 offset) override;
        bool write(const void *src, u32 size, u32 offset) override;
        bool sync() override;
        bool truncate(u64 size) override;
        bool size(u64 &size) override;
        Storage *storage() override;

        FATStorage *m_storage;
        FIL m_fFile;

        friend class FATStorage;
    };

    class Dir : public Storage::Dir {
    public:
        Dir();

    private:
        void close() override;
        bool read(NodeInfo &nodeInfo) override;
        Storage *storage() override;

        FATStorage *m_storage;
        DIR m_fDir;

        friend class FATStorage;
    };

    FATStorage(class Mutex *mutex);
    ~FATStorage();

    void remove();
    bool add();

    Storage::File *openFile(const char *path, u32 mode) override;

    Storage::Dir *openDir(const char *path) override;
    bool createDir(const char *path, u32 mode) override;

    bool rename(const char *srcPath, const char *dstPath) override;
    bool remove(const char *path, u32 mode) override;

    virtual u32 sectorSize() = 0;
    virtual bool read(u32 firstSector, u32 sectorCount, void *buffer) = 0;
    virtual bool write(u32 firstSector, u32 sectorCount, const void *buffer) = 0;
    virtual bool erase(u32 firstSector, u32 sectorCount) = 0;
    virtual bool sync() = 0;

    bool convertPath(const char *path, Array<char, 256> &fPath);

private:
    template <typename N>
    static N *FindNode(Array<N, 32> &nodes) {
        for (u32 i = 0; i < nodes.count(); i++) {
            if (!nodes[i].m_storage) {
                return &nodes[i];
            }
        }

        return nullptr;
    }

    u8 m_volumeId;
    FATFS m_fs;
    Array<File, 32> m_files;
    Array<Dir, 32> m_dirs;

    static Array<FATStorage *, FF_VOLUMES> s_volumes;

    friend DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count);
    friend DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count);
    friend DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff);
};
