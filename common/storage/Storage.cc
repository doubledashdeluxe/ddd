#include "Storage.hh"

#include "common/Log.hh"

#include <portable/Algorithm.hh>

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
    assert(mode == Mode::Read || mode == Mode::ReadWrite || mode == Mode::WriteAlways ||
            mode == Mode::WriteNew);

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

bool Storage::FileHandle::truncate(u64 size) {
    StorageHandle storage(*this);
    if (!m_file) {
        return false;
    }

    return m_file->truncate(size);
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

Storage::File::~File() {
    close();
}

void Storage::File::close() {
    if (m_handle) {
        FileHandle *handle = m_handle;
        m_handle = nullptr;
        handle->close();
    }
}

Storage::Dir::Dir() : m_handle(nullptr) {}

Storage::Dir::~Dir() {
    close();
}

void Storage::Dir::close() {
    if (m_handle) {
        DirHandle *handle = m_handle;
        m_handle = nullptr;
        handle->close();
    }
}

Storage::Observer::Observer() : m_next(s_headObserver) {
    assert(!s_head);
    s_headObserver = this;
}

Storage::Observer::~Observer() {
    Observer **next;
    for (next = &s_headObserver; *next != this; next = &(*next)->m_next) {}
    *next = m_next;
}

Storage::Observer *Storage::Observer::next() {
    return m_next;
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

    FileHandle file(path, mode);
    return file.write(src, size, 0);
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

Storage::Storage(Mutex *mutex) : m_next(nullptr), m_isContained(false), m_mutex(mutex) {}

Storage::~Storage() {}

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

void Storage::StorageHandle::acquireWithoutLocking(const char *path) {
    if (s_head && s_head->priority() > 0) {
        if (!strncmp(path, "main:", strlen("main:"))) {
            m_storage = s_head;
            m_prefix = "main:";
        }
    }
    if (!m_storage) {
        for (Storage *storage = s_head; storage; storage = storage->m_next) {
            if (!strncmp(path, storage->prefix(), strlen(storage->prefix()))) {
                m_storage = storage;
                m_prefix = storage->prefix();
                break;
            }
        }
    }
}

void Storage::StorageHandle::acquireWithoutLocking(const FileHandle &file) {
    if (file.m_file) {
        m_storage = file.m_file->storage();
    }
}

void Storage::StorageHandle::acquireWithoutLocking(const DirHandle &dir) {
    if (dir.m_dir) {
        m_storage = dir.m_dir->storage();
    }
}

void Storage::removeWithoutLocking() {
    bool hasOldMain = s_head == this && s_head->priority() > 0;
    m_isContained = false;
    Storage **next;
    for (next = &s_head; *next != this; next = &(*next)->m_next) {}
    *next = m_next;
    bool hasNewMain = s_head && s_head->priority() > 0;
    for (Observer *observer = s_headObserver; observer; observer = observer->next()) {
        if (hasOldMain) {
            observer->onRemove("main:");
            if (hasNewMain) {
                observer->onAdd("main:");
            }
        }
        observer->onRemove(prefix());
    }
}

void Storage::addWithoutLocking() {
    bool hasOldMain = s_head && s_head->priority() > 0;
    Storage **next;
    for (next = &s_head; *next && (*next)->priority() >= priority(); next = &(*next)->m_next) {}
    m_next = *next;
    *next = this;
    m_isContained = true;
    bool hasNewMain = s_head == this && s_head->priority() > 0;
    for (Observer *observer = s_headObserver; observer; observer = observer->next()) {
        observer->onAdd(prefix());
        if (hasNewMain) {
            if (hasOldMain) {
                observer->onRemove("main:");
            }
            observer->onAdd("main:");
        }
    }
}

Storage *Storage::s_head = nullptr;
Storage::Observer *Storage::s_headObserver = nullptr;
