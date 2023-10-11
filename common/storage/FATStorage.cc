#include "FATStorage.hh"

#include <common/Log.hh>
#include <common/Panic.hh>

extern "C" {
#include <assert.h>
#include <stdio.h>
#include <string.h>
}

FATStorage::File::File() : m_storage(nullptr) {}

void FATStorage::File::close() {
    f_close(&m_fFile);
    m_storage = nullptr;
}

bool FATStorage::File::read(void *dst, u32 size, u32 offset) {
    if (f_lseek(&m_fFile, offset) != FR_OK) {
        return false;
    }

    UINT readSize;
    if (f_read(&m_fFile, dst, size, &readSize) != FR_OK) {
        return false;
    }

    return readSize == size;
}

bool FATStorage::File::write(const void *src, u32 size, u32 offset) {
    if (f_lseek(&m_fFile, offset) != FR_OK) {
        return false;
    }

    UINT writtenSize;
    if (f_write(&m_fFile, src, size, &writtenSize) != FR_OK) {
        return false;
    }

    return writtenSize == size;
}

bool FATStorage::File::sync() {
    return f_sync(&m_fFile) == FR_OK;
}

bool FATStorage::File::size(u64 &size) {
    size = f_size(&m_fFile);
    return true;
}

Storage *FATStorage::File::storage() {
    return m_storage;
}

FATStorage::Dir::Dir() : m_storage(nullptr) {}

void FATStorage::Dir::close() {
    f_closedir(&m_fDir);
    m_storage = nullptr;
}

bool FATStorage::Dir::read(NodeInfo &nodeInfo) {
    FILINFO fInfo;
    if (f_readdir(&m_fDir, &fInfo) != FR_OK) {
        return false;
    }

    if (fInfo.fname[0] == L'\0') {
        return false;
    }

    if (fInfo.fattrib & AM_DIR) {
        nodeInfo.type = NodeType::Dir;
    } else {
        nodeInfo.type = NodeType::File;
    }
    static_assert(sizeof(fInfo.fname) <= sizeof(nodeInfo.name));
    memcpy(nodeInfo.name.values(), fInfo.fname, sizeof(fInfo.fname));
    return true;
}

Storage *FATStorage::Dir::storage() {
    return m_storage;
}

FATStorage::FATStorage() {}

void FATStorage::remove() {
    for (u32 i = 0; i < m_files.count(); i++) {
        m_files[i].close();
    }
    for (u32 i = 0; i < m_dirs.count(); i++) {
        m_dirs[i].close();
    }

    Array<char, 256> fVolumePath;
    snprintf(fVolumePath.values(), fVolumePath.count(), "%u:", m_volumeId);
    f_unmount(fVolumePath.values());

    s_volumes[m_volumeId] = nullptr;

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

    Array<char, 256> fVolumePath;
    snprintf(fVolumePath.values(), fVolumePath.count(), "%u:", m_volumeId);
    FRESULT fResult = f_mount(&m_fs, fVolumePath.values(), 1);
    if (fResult != FR_OK) {
        DEBUG("Failed to mount the filesystem with error %u\n", fResult);
        s_volumes[m_volumeId] = nullptr;
        return false;
    }

    Array<char, 256> fDirPath;
    snprintf(fDirPath.values(), fDirPath.count(), "%u:/ddd", m_volumeId);
    fResult = f_mkdir(fDirPath.values());
    if (fResult != FR_OK && fResult != FR_EXIST) {
        DEBUG("Failed to create or open the %s directory with error %u", fDirPath.values(),
                fResult);
        f_unmount(fVolumePath.values());
        s_volumes[m_volumeId] = nullptr;
        return false;
    }

    Storage::add();
    return true;
}

Storage::File *FATStorage::openFile(const char *path, u32 mode) {
    Array<char, 256> fPath;
    if (!convertPath(path, fPath)) {
        return nullptr;
    }
    u32 fMode;
    switch (mode) {
    case Mode::Read:
        fMode = FA_READ;
        break;
    case Mode::WriteAlways:
        fMode = FA_CREATE_ALWAYS | FA_WRITE;
        break;
    case Mode::WriteNew:
        fMode = FA_CREATE_NEW | FA_WRITE;
        break;
    default:
        return nullptr;
    }

    File *file = FindNode(m_files);
    if (!file) {
        return nullptr;
    }

    if (f_open(&file->m_fFile, fPath.values(), fMode) != FR_OK) {
        return nullptr;
    }

    file->m_storage = this;
    return file;
}

Storage::Dir *FATStorage::openDir(const char *path) {
    Array<char, 256> fPath;
    if (!convertPath(path, fPath)) {
        return nullptr;
    }

    Dir *dir = FindNode(m_dirs);
    if (!dir) {
        return nullptr;
    }

    if (f_opendir(&dir->m_fDir, fPath.values()) != FR_OK) {
        return nullptr;
    }

    dir->m_storage = this;
    return dir;
}

bool FATStorage::createDir(const char *path, u32 mode) {
    Array<char, 256> fPath;
    if (!convertPath(path, fPath)) {
        return false;
    }

    FRESULT fResult = f_mkdir(fPath.values());
    return fResult == FR_OK || (mode == Mode::WriteAlways && fResult == FR_EXIST);
}

bool FATStorage::rename(const char *srcPath, const char *dstPath) {
    Array<char, 256> fSrcPath;
    if (!convertPath(srcPath, fSrcPath)) {
        return false;
    }

    Array<char, 256> fDstPath;
    if (!convertPath(dstPath, fDstPath)) {
        return false;
    }

    FRESULT fResult = f_rename(fSrcPath.values(), fDstPath.values());
    return fResult == FR_OK;
}

bool FATStorage::remove(const char *path, u32 mode) {
    Array<char, 256> fPath;
    if (!convertPath(path, fPath)) {
        return false;
    }

    FRESULT fResult = f_unlink(path);
    return fResult == FR_OK || (mode == Mode::RemoveAlways && fResult == FR_NO_FILE);
}

bool FATStorage::convertPath(const char *path, Array<char, 256> &fPath) {
    return snprintf(fPath.values(), fPath.count(), "%u:/%s", m_volumeId, path) < fPath.count();
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
