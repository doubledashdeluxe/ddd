#include "DVDStorage.hh"

#include <common/Algorithm.hh>
#include <common/Align.hh>
#include <common/Arena.hh>
#include <common/Memory.hh>

extern "C" {
#include <assert.h>
#include <stdio.h>
#include <string.h>
}

void DVDStorage::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x20) DVDStorage;
}

DVDStorage::File::File() : m_storage(nullptr) {}

void DVDStorage::File::close() {
    DVDClose(&m_fileInfo);
    m_storage = nullptr;
    Storage::File::close();
}

bool DVDStorage::File::read(void *dst, u32 size, u32 offset) {
    assert(IsAligned(size, 0x20));
    assert(IsAligned(offset, 0x20));

    if (Memory::IsMEM1(dst) && Memory::IsAligned(dst, 0x20)) {
        return DVDReadPrio(&m_fileInfo, dst, size, offset, 2);
    } else {
        while (size > 0) {
            u32 chunkSize = Min<u32>(size, m_storage->m_buffer.count());
            if (!DVDReadPrio(&m_fileInfo, m_storage->m_buffer.values(), chunkSize, offset, 2)) {
                return false;
            }
            memcpy(dst, m_storage->m_buffer.values(), chunkSize);
            dst = reinterpret_cast<u8 *>(dst) + chunkSize;
            size -= chunkSize;
            offset += chunkSize;
        }
        return true;
    }
}

bool DVDStorage::File::write(const void * /* src */, u32 /* size */, u32 /* offset */) {
    return false;
}

bool DVDStorage::File::sync() {
    return false;
}

bool DVDStorage::File::truncate(u64 /* size */) {
    return false;
}

bool DVDStorage::File::size(u64 &size) {
    size = AlignUp(m_fileInfo.length, 0x20);
    return true;
}

Storage *DVDStorage::File::storage() {
    return m_storage;
}

DVDStorage::Dir::Dir() : m_storage(nullptr) {}

void DVDStorage::Dir::close() {
    DVDCloseDir(&m_dir);
    m_storage = nullptr;
    Storage::Dir::close();
}

bool DVDStorage::Dir::read(NodeInfo &nodeInfo) {
    DVDDirEntry entry;
    if (!DVDReadDir(&m_dir, &entry)) {
        return false;
    }

    if (entry.isDir) {
        nodeInfo.type = NodeType::Dir;
    } else {
        nodeInfo.type = NodeType::File;
    }
    snprintf(nodeInfo.name.values(), nodeInfo.name.count(), "%s", entry.name);
    return true;
}

Storage *DVDStorage::Dir::storage() {
    return m_storage;
}

DVDStorage::DVDStorage() : Storage(&m_mutex) {
    OSInitMessageQueue(&m_initQueue, m_initMessages.values(), m_initMessages.count());
    notify();
    OSReceiveMessage(&m_initQueue, nullptr, OS_MESSAGE_BLOCK);
}

void DVDStorage::poll() {
    add();
    OSSendMessage(&m_initQueue, nullptr, OS_MESSAGE_NOBLOCK);
}

u32 DVDStorage::priority() {
    return 0;
}

const char *DVDStorage::prefix() {
    return "dvd:";
}

Storage::File *DVDStorage::openFile(const char *path, u32 mode) {
    if (mode != Mode::Read) {
        return nullptr;
    }

    File *file = FindNode(m_files);
    if (!file) {
        return nullptr;
    }

    if (!DVDOpen(path, &file->m_fileInfo)) {
        return nullptr;
    }

    file->m_storage = this;
    return file;
}

Storage::Dir *DVDStorage::openDir(const char *path) {
    Dir *dir = FindNode(m_dirs);
    if (!dir) {
        return nullptr;
    }

    if (!DVDOpenDir(path, &dir->m_dir)) {
        return nullptr;
    }

    dir->m_storage = this;
    return dir;
}

bool DVDStorage::createDir(const char * /* path */, u32 /* mode */) {
    return false;
}

bool DVDStorage::rename(const char * /* srcPath */, const char * /* dstPath */) {
    return false;
}

bool DVDStorage::remove(const char * /* path */, u32 /* mode */) {
    return false;
}

DVDStorage *DVDStorage::s_instance = nullptr;
