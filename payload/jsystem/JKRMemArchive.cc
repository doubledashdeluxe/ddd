#include "JKRMemArchive.hh"

#include <common/Bytes.hh>
#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/DVD.h>

#include <stdio.h>
}

bool JKRMemArchive::open(s32 entrynum, u32 mountDirection) {
    m_mountMode = MountMode::None;
    m_mountDirection = mountDirection;
    m_ownsMemory = false;

    Array<char, 256> path;
    if (!DVDConvertEntrynumToPath(entrynum, path.values(), path.count())) {
        return false;
    }

    Array<char, 256> filePath;
    snprintf(filePath.values(), filePath.count(), "dvd:%s", path.values());
    Storage::FileHandle file(filePath.values(), Storage::Mode::Read);

    u64 fileSize;
    if (!file.size(fileSize)) {
        return false;
    }
    if (fileSize > UINT32_MAX) {
        return false;
    }

    s32 align = mountDirection == MountDirection::Head ? 0x20 : -0x20;
    m_archive = m_heap->alloc(fileSize, align);
    if (!m_archive) {
        return false;
    }

    u8 *header = reinterpret_cast<u8 *>(m_archive);
    if (!file.read(header, 0x20, 0x0)) {
        m_heap->free(m_archive);
        return false;
    }

    u32 treeOffset = Bytes::ReadBE<u32>(header, 0x08);
    u32 treeSize = Bytes::ReadBE<u32>(header, 0x0c);
    if (treeOffset > fileSize || treeSize > fileSize || treeOffset + treeSize > fileSize) {
        m_heap->free(m_archive);
        return false;
    }
    u32 filesOffset = treeOffset + treeSize;
    u32 filesSize = fileSize - filesOffset;
    m_tree = header + 0x20 + filesSize;
    if (!file.read(m_tree, treeSize, treeOffset)) {
        m_heap->free(m_archive);
        return false;
    }
    m_files = header + 0x20;
    if (!file.read(m_files, filesSize, filesOffset)) {
        m_heap->free(m_archive);
        return false;
    }

    u32 dirsOffset = Bytes::ReadBE<u32>(m_tree, 0x04);
    u32 dirsSize = Bytes::ReadBE<u32>(m_tree, 0x00) * 0x10;
    if (dirsOffset > treeSize || dirsSize > treeSize || dirsOffset + treeOffset > treeSize) {
        m_heap->free(m_archive);
        return false;
    }
    m_dirs = m_tree + dirsOffset;

    u32 nodesOffset = Bytes::ReadBE<u32>(m_tree, 0x0c);
    u32 nodesSize = Bytes::ReadBE<u32>(m_tree, 0x08) * 0x14;
    if (nodesOffset > treeSize || nodesSize > treeSize || nodesOffset + nodesSize > treeSize) {
        m_heap->free(m_archive);
        return false;
    }
    m_nodes = m_tree + nodesOffset;

    u32 namesOffset = Bytes::ReadBE<u32>(m_tree, 0x14);
    u32 namesSize = Bytes::ReadBE<u32>(m_tree, 0x10);
    if (namesOffset > treeSize || namesSize > treeSize || namesOffset + namesSize > treeSize) {
        m_heap->free(m_archive);
        return false;
    }
    m_names = reinterpret_cast<char *>(m_tree + namesOffset);

    m_mountMode = MountMode::Mem;
    m_ownsMemory = true;

    return true;
}
