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
    FATStorage();

    void remove();
    bool add();

    virtual u32 sectorSize() = 0;
    virtual bool read(u32 firstSector, u32 sectorCount, void *buffer) = 0;
    virtual bool write(u32 firstSector, u32 sectorCount, const void *buffer) = 0;
    virtual bool erase(u32 firstSector, u32 sectorCount) = 0;
    virtual bool sync() = 0;

private:
    ~FATStorage();

    u8 m_volumeId;
    FATFS m_fs;

    static Array<FATStorage *, FF_VOLUMES> s_volumes;
#ifdef PAYLOAD
    static Mutex s_mutex;
#endif

    friend DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count);
    friend DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count);
    friend DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff);
};
