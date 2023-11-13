#include "Storage.hh"

#include "common/Algorithm.hh"
#include "common/Log.hh"

extern "C" {
#include <assert.h>
#include <stdio.h>
#include <string.h>
}

Storage::FileHandle::FileHandle() : m_file(nullptr) {}

Storage::FileHandle::FileHandle(const char *path, u32 mode) : m_file(nullptr) {
    open(path, mode);
}

Storage::FileHandle::~FileHandle() {
    close();
}

bool Storage::FileHandle::open(const char *path, u32 mode) {
    assert(path);
    assert(mode == Mode::Read || mode == Mode::WriteAlways || mode == Mode::WriteNew);

    close();

    StorageHandle storage(path);
    m_file = storage.openFile(path + strlen(storage.prefix()), mode);
    if (m_file) {
        m_file->m_handle = this;
    }
    return m_file;
}

void Storage::FileHandle::close() {
    StorageHandle storage(*this);
    if (m_file) {
        File *file = m_file;
        m_file = nullptr;
        file->close();
    }
}

bool Storage::FileHandle::read(void *dst, u32 size, u32 offset) {
    assert(dst);

    StorageHandle storage(*this);
    if (!m_file) {
        return false;
    }

    return m_file->read(dst, size, offset);
}

bool Storage::FileHandle::write(const void *src, u32 size, u32 offset) {
    assert(src);

    StorageHandle storage(*this);
    if (!m_file) {
        return false;
    }

    return m_file->write(src, size, offset);
}

bool Storage::FileHandle::sync() {
    StorageHandle storage(*this);
    if (!m_file) {
        return false;
    }

    return m_file->sync();
}

bool Storage::FileHandle::size(u64 &size) {
    StorageHandle storage(*this);
    if (!m_file) {
        return false;
    }

    return m_file->size(size);
}

Storage::DirHandle::DirHandle() : m_dir(nullptr) {}

Storage::DirHandle::DirHandle(const char *path) : m_dir(nullptr) {
    open(path);
}

Storage::DirHandle::~DirHandle() {
    close();
}

bool Storage::DirHandle::open(const char *path) {
    assert(path);

    close();

    StorageHandle storage(path);
    m_dir = storage.openDir(path + strlen(storage.prefix()));
    if (m_dir) {
        m_dir->m_handle = this;
    }
    return m_dir;
}

void Storage::DirHandle::close() {
    StorageHandle storage(*this);
    if (m_dir) {
        Dir *dir = m_dir;
        m_dir = nullptr;
        dir->close();
    }
}

bool Storage::DirHandle::read(NodeInfo &nodeInfo) {
    StorageHandle storage(*this);
    if (!m_dir) {
        return false;
    }

    return m_dir->read(nodeInfo);
}

Storage::File::File() : m_handle(nullptr) {}

void Storage::File::close() {
    if (m_handle) {
        FileHandle *handle = m_handle;
        m_handle = nullptr;
        handle->close();
    }
}

Storage::Dir::Dir() : m_handle(nullptr) {}

void Storage::Dir::close() {
    if (m_handle) {
        DirHandle *handle = m_handle;
        m_handle = nullptr;
        handle->close();
    }
}

bool Storage::ReadFile(const char *path, void *dst, u32 size, u32 *readSize) {
    FileHandle file(path, Mode::Read);
    u64 fileSize;
    if (!file.size(fileSize)) {
        return false;
    }
    size = Min(static_cast<u64>(size), fileSize);
    if (!file.read(dst, size, 0)) {
        return false;
    }
    if (readSize) {
        *readSize = size;
    }
    return true;
}

bool Storage::WriteFile(const char *path, const void *src, u32 size, u32 mode) {
    assert(mode == Mode::WriteAlways || mode == Mode::WriteNew);

    if (mode == Mode::WriteAlways) {
        StorageHandle storage(path);

        Array<char, 256> newPath;
        snprintf(newPath.values(), newPath.count(), "%s.new", path);
        Array<char, 256> oldPath;
        snprintf(oldPath.values(), oldPath.count(), "%s.old", path);

        {
            FileHandle file(newPath.values(), Mode::WriteAlways);
            if (!file.write(src, size, 0)) {
                DEBUG("Failed to write to %s\n", newPath.values());
                return false;
            }
        }

        if (!Remove(oldPath.values(), true)) {
            DEBUG("Failed to remove %s\n", oldPath.values());
            return false;
        }

        if (!Rename(path, oldPath.values())) {
            DEBUG("Failed to rename %s to %s\n", path, oldPath.values());
            // Ignore
        }

        if (!Rename(newPath.values(), path)) {
            DEBUG("Failed to rename %s to %s\n", newPath.values(), path);
            return false;
        }

        Remove(oldPath.values(), true); // Not a big deal if this fails
        return true;
    } else {
        FileHandle file(path, mode);
        return file.write(src, size, 0);
    }
}

bool Storage::CreateDir(const char *path, u32 mode) {
    assert(path);
    assert(mode == Mode::WriteAlways || mode == Mode::WriteNew);

    StorageHandle storage(path);
    return storage.createDir(path + strlen(storage.prefix()), mode);
}

bool Storage::Rename(const char *srcPath, const char *dstPath) {
    assert(srcPath);
    assert(dstPath);

    StorageHandle storage(srcPath);
    if (strlen(storage.prefix()) == 0) {
        return false;
    }
    if (strncmp(dstPath, storage.prefix(), strlen(storage.prefix()))) {
        return false;
    }

    return storage.rename(srcPath + strlen(storage.prefix()), dstPath + strlen(storage.prefix()));
}

bool Storage::Remove(const char *path, u32 mode) {
    assert(path);
    assert(mode == Mode::RemoveAlways || mode == Mode::RemoveExisting);

    StorageHandle storage(path);
    return storage.remove(path + strlen(storage.prefix()), mode);
}

bool Storage::isContained() const {
    return m_isContained;
}

const char *Storage::StorageHandle::prefix() {
    return m_prefix ? m_prefix : "";
}

Storage::File *Storage::StorageHandle::openFile(const char *path, u32 mode) {
    if (!m_storage) {
        return nullptr;
    }

    return m_storage->openFile(path, mode);
}

Storage::Dir *Storage::StorageHandle::openDir(const char *path) {
    if (!m_storage) {
        return nullptr;
    }

    return m_storage->openDir(path);
}

bool Storage::StorageHandle::createDir(const char *path, u32 mode) {
    if (!m_storage) {
        return false;
    }

    return m_storage->createDir(path, mode);
}

bool Storage::StorageHandle::rename(const char *srcPath, const char *dstPath) {
    if (!m_storage) {
        return false;
    }

    return m_storage->rename(srcPath, dstPath);
}

bool Storage::StorageHandle::remove(const char *path, u32 mode) {
    if (!m_storage) {
        return false;
    }

    return m_storage->remove(path, mode);
}

Storage *Storage::s_head = nullptr;
