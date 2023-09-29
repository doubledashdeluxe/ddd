#include "FATStorage.hh"

#include <common/Log.hh>
#include <common/Panic.hh>

extern "C" {
#include <stdio.h>
}

FATStorage::FATStorage() {}

void FATStorage::remove() {
    Storage::remove();
}

bool FATStorage::add() {
    for (m_volumeId = 0; m_volumeId < s_volumes.count(); m_volumeId++) {
        if (!s_volumes[m_volumeId]) {
            break;
        }
    }
    if (m_volumeId == s_volumes.count()) {
        return false;
    }
    s_volumes[m_volumeId] = this;

    FRESULT fResult = f_mount(&m_fs, "", 1);
    if (fResult != FR_OK) {
        DEBUG("Failed to mount the filesystem with error %u\n", fResult);
        s_volumes[m_volumeId] = nullptr;
        return false;
    }

    Array<char, 128> fPath;
    snprintf(fPath.values(), fPath.count(), "%u:/ddd", m_volumeId);
    fResult = f_mkdir(fPath.values());
    if (fResult != FR_OK && fResult != FR_EXIST) {
        DEBUG("Failed to create or open the %s directory with error %u", fPath.values(), fResult);
        s_volumes[m_volumeId] = nullptr;
        return false;
    }

    Storage::add();
    return true;
}

extern "C" DSTATUS disk_status(BYTE /* pdrv */) {
    return 0;
}

extern "C" DSTATUS disk_initialize(BYTE /* pdrv */) {
    return 0;
}

extern "C" DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    return FATStorage::s_volumes[pdrv]->read(sector, count, buff) ? RES_OK : RES_ERROR;
}

extern "C" DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    return FATStorage::s_volumes[pdrv]->write(sector, count, buff) ? RES_OK : RES_ERROR;
}

extern "C" DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    switch (cmd) {
    case CTRL_SYNC:
        return FATStorage::s_volumes[pdrv]->sync() ? RES_OK : RES_ERROR;
    case GET_SECTOR_SIZE:
        *reinterpret_cast<WORD *>(buff) = FATStorage::s_volumes[pdrv]->sectorSize();
        return RES_OK;
    case CTRL_TRIM: {
        const LBA_t *args = reinterpret_cast<LBA_t *>(buff);
        LBA_t firstSector = args[0];
        LBA_t sectorCount = args[1] - args[0] + 1;
        return FATStorage::s_volumes[pdrv]->erase(firstSector, sectorCount) ? RES_OK : RES_ERROR;
    }
    default:
        PANIC("Invalid ioctl");
    }
}
