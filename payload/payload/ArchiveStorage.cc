#include "ArchiveStorage.hh"

#include <common/Arena.hh>
#include <common/Bytes.hh>
extern "C" {
#include <miniz/miniz.h>

#include <stdio.h>
#include <string.h>
}

void ArchiveStorage::Init(const char *prefix, void *archive, u32 archiveSize) {
    s_head = new (MEM2Arena::Instance(), 0x20) ArchiveStorage(prefix, archive, archiveSize);
}

ArchiveStorage::File::File() : m_storage(nullptr), m_node(nullptr) {}

void ArchiveStorage::File::close() {
    m_storage = nullptr;
    Storage::File::close();
}

bool ArchiveStorage::File::read(void *dst, u32 size, u32 offset) {
    const u8 *file = m_node.getFile(m_storage->m_archive.getFiles());
    if (size > Bytes::ReadBE<u32>(file, 0) || offset != 0) {
        return false;
    }

    u32 fileSize = m_node.getFileSize();
    return tinfl_decompress_mem_to_mem(dst, size, file + 4, fileSize - 4, 0) == size;
}

bool ArchiveStorage::File::write(const void * /* src */, u32 /* size */, u32 /* offset */) {
    return false;
}

bool ArchiveStorage::File::sync() {
    return false;
}

bool ArchiveStorage::File::truncate(u64 /* size */) {
    return false;
}

bool ArchiveStorage::File::size(u64 &size) {
    const u8 *file = m_node.getFile(m_storage->m_archive.getFiles());
    size = Bytes::ReadBE<u32>(file, 0);
    return true;
}

Storage *ArchiveStorage::File::storage() {
    return m_storage;
}

ArchiveStorage::Dir::Dir() : m_storage(nullptr), m_dir(nullptr) {}

void ArchiveStorage::Dir::close() {
    m_storage = nullptr;
    Storage::Dir::close();
}

bool ArchiveStorage::Dir::read(NodeInfo &nodeInfo) {
    if (m_index >= m_dir.getNodeCount()) {
        return false;
    }
    Archive::Node node = m_dir.getNode(m_index++, m_storage->m_archive.getTree());

    if (node.isDir()) {
        nodeInfo.type = NodeType::Dir;
    } else {
        nodeInfo.type = NodeType::File;
    }
    const char *name = node.getName(m_storage->m_archive.getTree().getNames());
    snprintf(nodeInfo.name.values(), nodeInfo.name.count(), "%s", name);
    return true;
}

Storage *ArchiveStorage::Dir::storage() {
    return m_storage;
}

ArchiveStorage::ArchiveStorage(const char *prefix, void *archive, u32 archiveSize)
    : Storage(&m_mutex), m_next(s_head), m_prefix(prefix),
      m_archive(reinterpret_cast<u8 *>(archive)), m_archiveSize(archiveSize) {
    if (m_archive.isValid(m_archiveSize)) {
        OSInitMessageQueue(&m_initQueue, m_initMessages.values(), m_initMessages.count());
        notify();
        OSReceiveMessage(&m_initQueue, nullptr, OS_MESSAGE_BLOCK);
    }
}

void ArchiveStorage::poll() {
    add();
    OSSendMessage(&m_initQueue, nullptr, OS_MESSAGE_NOBLOCK);
}

u32 ArchiveStorage::priority() {
    return 0;
}

const char *ArchiveStorage::prefix() {
    return m_prefix;
}

Storage::File *ArchiveStorage::openFile(const char *path, u32 mode) {
    if (mode != Mode::Read) {
        return nullptr;
    }

    File *file = FindNode(m_files);
    if (!file) {
        return nullptr;
    }

    const char *name;
    Archive::Dir dir(nullptr);
    bool exists;
    if (!m_archive.getTree().search(path, name, dir, file->m_node, exists)) {
        return nullptr;
    }
    if (!exists) {
        return nullptr;
    }
    if (!file->m_node.isFile()) {
        return nullptr;
    }
    if (file->m_node.getFileSize() < 4) {
        return nullptr;
    }

    file->m_storage = this;
    return file;
}

Storage::Dir *ArchiveStorage::openDir(const char *path) {
    Dir *dir = FindNode(m_dirs);
    if (!dir) {
        return nullptr;
    }

    const char *name;
    Archive::Node node(nullptr);
    bool exists;
    if (!m_archive.getTree().search(path, name, dir->m_dir, node, exists)) {
        return nullptr;
    }
    if (!exists) {
        return nullptr;
    }
    if (!node.isDir()) {
        return nullptr;
    }
    dir->m_index = 0;

    dir->m_storage = this;
    return dir;
}

bool ArchiveStorage::createDir(const char * /* path */, u32 /* mode */) {
    return false;
}

bool ArchiveStorage::rename(const char * /* srcPath */, const char * /* dstPath */) {
    return false;
}

bool ArchiveStorage::remove(const char * /* path */, u32 /* mode */) {
    return false;
}

ArchiveStorage *ArchiveStorage::s_head = nullptr;
