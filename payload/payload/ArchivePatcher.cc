#include "ArchivePatcher.hh"

#include <common/Algorithm.hh>
#include <common/Align.hh>
#include <common/Bytes.hh>
#include <common/DCache.hh>
#include <common/Log.hh>

extern "C" {
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
}

bool ArchivePatcher::Patch(const char *bare, Archive &archive, u32 archiveSize, JKRHeap *heap,
        s32 alignment, bool &ownsMemory) {
    ArchivePatcher patcher(bare, archive, archiveSize, heap, alignment, ownsMemory);
    if (!patcher.patch()) {
        if (ownsMemory) {
            delete[] archive.get();
        }
        return false;
    }
    return true;
}

ArchivePatcher::ArchivePatcher(const char *bare, Archive &archive, u32 archiveSize, JKRHeap *heap,
        s32 alignment, bool &ownsMemory)
    : m_bare(bare), m_archive(archive), m_archiveSize(archiveSize), m_heap(heap),
      m_alignment(alignment), m_ownsMemory(ownsMemory) {
    if (m_ownsMemory) {
        u8 *tree = archive.getTree().get();
        u32 treeSize = archive.getTreeSize();
        u32 filesSize = archive.getFilesSize();
        RotateLeft(tree, treeSize + filesSize, treeSize);
        u32 treeOffset = archive.getTreeOffset();
        archive.setTreeOffset(treeOffset + filesSize);
        archive.setFilesOffset(treeOffset);
    }
}

ArchivePatcher::~ArchivePatcher() {}

bool ArchivePatcher::patch() {
    if (!addDir("carc:/")) {
        return false;
    }
    if (!addDir("larc:/")) {
        return false;
    }
    if (!addDir("main:/ddd/assets/")) {
        return false;
    }
    shrink();
    m_archive.setArchiveSize(m_archiveSize);
    return true;
}

bool ArchivePatcher::addDir(const char *prefix) {
    Array<char, 256> path;
    s32 rootLength = snprintf(path.values(), path.count(), "%s%s", prefix, m_bare);
    assert(rootLength >= 0 && static_cast<u32>(rootLength) < path.count());
    Storage::NodeInfo nodeInfo;
    return addDir(rootLength, path, nodeInfo);
}

bool ArchivePatcher::addDir(u32 rootLength, Array<char, 256> &path, Storage::NodeInfo &nodeInfo) {
    const char *searchPath = path.values() + rootLength;
    if (searchPath[0] == '\0') {
        searchPath = "/";
    }
    const char *name;
    Archive::Dir dir(nullptr);
    Archive::Node node(nullptr);
    bool exists;
    if (!m_archive.getTree().search(searchPath, name, dir, node, exists)) {
        return false;
    }
    if (exists) {
        if (node.get() && !node.isDir()) {
            return false;
        }
    } else {
        u32 archiveSize = m_ownsMemory ? m_archiveSize : 0;

        Archive::Tree tree = m_archive.getTree();
        u32 dirIndex = (dir.get() - tree.getDir(0).get()) / 0x10;
        u32 nodeIndex = (node.get() - tree.getNode(0).get()) / 0x14;

        if (!addNode(0x10, name, dirIndex, nodeIndex)) {
            return false;
        }
        tree = m_archive.getTree();
        dirIndex = tree.getDirCount();
        node = tree.getNode(nodeIndex);
        node.setFileIndex(UINT16_MAX);
        node.setType(0x02);
        node.setDirIndex(dirIndex);

        tree.setDirCount(tree.getDirCount() + 1);
        dir = tree.getDir(dirIndex);
        Array<u8, 4> type;
        for (u32 i = 0; i < type.count(); i++) {
            if (name[i] == '\0') {
                for (u32 j = i; j < type.count(); j++) {
                    type[j] = ' ';
                }
                break;
            } else {
                type[i] = toupper(name[i]);
            }
        }
        dir.setType(Bytes::ReadBE<u32>(type.values(), 0x0));
        dir.setNameOffset(node.getNameOffset());
        dir.setNameHash(node.getNameHash());
        dir.setNodeCount(0);
        dir.setFirstNodeIndex(tree.getNodeCount());

        u32 size = m_archiveSize - archiveSize;
        DEBUG("Added %s to %s (0x%lx)", searchPath, m_bare, size);
    }

    u32 length = strlen(path.values());
    for (Storage::DirHandle dir(path.values()); dir.read(nodeInfo);) {
        snprintf(path.values() + length, path.count() - length, "/%s", nodeInfo.name.values());
        if (nodeInfo.type == Storage::NodeType::Dir) {
            if (!addDir(rootLength, path, nodeInfo)) {
                return false;
            }
        } else {
            if (!addFile(rootLength, path)) {
                return false;
            }
        }
    }
    return true;
}

bool ArchivePatcher::addFile(u32 rootLength, Array<char, 256> &path) {
    u32 archiveSize = m_ownsMemory ? m_archiveSize : 0;

    Storage::FileHandle file(path.values(), Storage::Mode::Read);
    u64 fileSize;
    if (!file.size(fileSize)) {
        return false;
    }
    u64 alignedFileSize = AlignUp(fileSize, 0x20);
    if (alignedFileSize > UINT32_MAX) {
        return false;
    }

    Archive::Tree tree = m_archive.getTree();
    const char *searchPath = path.values() + rootLength;
    if (searchPath[0] == '\0') {
        searchPath = "/";
    }
    const char *name;
    Archive::Dir dir(nullptr);
    Archive::Node node(nullptr);
    bool exists;
    if (!tree.search(searchPath, name, dir, node, exists)) {
        return false;
    }
    if (!node.get()) {
        return false;
    }
    u32 nodeIndex = (node.get() - tree.getNode(0).get()) / 0x14;
    if (exists) {
        if (!node.isFile()) {
            return false;
        }

        if (!m_ownsMemory || alignedFileSize > node.getFileSize()) {
            if (!grow(alignedFileSize)) {
                return false;
            }
            tree = Archive::Tree(m_archive.getFiles() + m_archive.getFilesSize() + alignedFileSize);
            u32 treeSize = m_archiveSize - alignedFileSize - m_archive.getTreeOffset();
            memmove(tree.get(), m_archive.getTree().get(), treeSize);
            m_archive.setTree(tree);

            node = tree.getNode(nodeIndex);
            node.setFileOffset(m_archive.getFilesSize());
            m_archive.setFilesSize(m_archive.getFilesSize() + alignedFileSize);
        }
    } else {
        u32 dirIndex = (dir.get() - tree.getDir(0).get()) / 0x10;
        u32 nodesOffset = tree.getNode(0).get() - tree.get();

        if (!addNode(alignedFileSize, name, dirIndex, nodeIndex)) {
            return false;
        }
        tree = m_archive.getTree();
        node = tree.getNode(nodeIndex);
        node.setFileIndex(tree.getFileCount());
        node.setType(0x11);
        node.setFileOffset(m_archive.getFilesSize());

        u8 *nodes = tree.getNode(0).get();
        char *names = tree.getNames();
        tree = Archive::Tree(m_archive.getFiles() + m_archive.getFilesSize() + alignedFileSize);
        memmove(tree.get(), m_archive.getTree().get(), nodesOffset);
        tree.setNodes(nodes);
        tree.setNames(names);
        m_archive.setTree(tree);
        tree.setFileCount(tree.getFileCount() + 1);

        node = tree.getNode(nodeIndex);
        m_archive.setFilesSize(m_archive.getFilesSize() + alignedFileSize);
    }
    node.setFileSize(fileSize);

    u8 *files = m_archive.getFiles();
    if (!file.read(node.getFile(files), fileSize, 0x0)) {
        return false;
    }
    DCache::Flush(node.getFile(files), fileSize);

    u32 size = m_archiveSize - archiveSize;
    DEBUG("Added %s to %s (0x%lx)", searchPath, m_bare, size);
    return true;
}

bool ArchivePatcher::addNode(u32 fileSize, const char *name, u32 dirIndex, u32 nodeIndex) {
    u32 nameLength = strlen(name);
    u32 nameSize = AlignUp(nameLength + 1, 0x4);
    u16 nameHash = 0;
    for (u32 i = 0; i < nameLength; i++) {
        nameHash *= 3;
        nameHash += name[i];
    }

    if (!grow(fileSize + 0x14 + nameSize)) {
        return false;
    }
    Archive::Tree tree = m_archive.getTree();

    u32 nameOffset = tree.getNamesSize();
    char *names = tree.getNames() + fileSize + 0x14;
    memmove(names, tree.getNames(), nameOffset);
    tree.setNames(names);
    tree.setNamesSize(nameOffset + nameSize);
    memcpy(names + nameOffset, name, nameLength + 1);

    u32 nodeOffset = nodeIndex * 0x14;
    u8 *nodes = tree.getNode(0).get() + fileSize;
    memmove(nodes + nodeOffset + 0x14, tree.getNode(nodeIndex).get(),
            (tree.getNodeCount() - nodeIndex) * 0x14);
    memmove(nodes, tree.getNode(0).get(), nodeOffset);
    tree.setNodes(nodes);
    tree.setNodeCount(tree.getNodeCount() + 1);
    Archive::Node node = tree.getNode(nodeIndex);
    node.setNameHash(nameHash);
    node.setNameOffset(nameOffset);
    node.setFileSize(fileSize);
    node.setFileAddress(0x0);

    for (u32 i = 0; i < tree.getDirCount(); i++) {
        if (i == dirIndex) {
            continue;
        }
        Archive::Dir dir = tree.getDir(i);
        u32 firstNodeIndex = dir.getFirstNodeIndex();
        if (firstNodeIndex >= nodeIndex) {
            dir.setFirstNodeIndex(firstNodeIndex + 1);
        }
    }

    Archive::Dir dir = tree.getDir(dirIndex);
    dir.setNodeCount(dir.getNodeCount() + 1);
    return true;
}

bool ArchivePatcher::grow(u32 size) {
    if (m_ownsMemory) {
        u32 archiveSize = m_archiveSize + size;
        if (archiveSize < m_archiveSize) {
            return false;
        }
        if (m_heap->resize(m_archive.get(), archiveSize) < 0) {
            Archive archive(new (m_heap, m_alignment) u8[archiveSize]);
            if (!archive.get()) {
                return false;
            }
            memcpy(archive.get(), m_archive.get(), m_archiveSize);
            delete[] m_archive.get();
            m_archive = archive;
        }
        m_archiveSize = archiveSize;
    } else {
        u32 archiveSize = 0x20 + m_archive.getTreeSize() + size;
        Archive archive(new (m_heap, m_alignment) u8[archiveSize]);
        if (!archive.get()) {
            return false;
        }
        memcpy(archive.get(), m_archive.get(), 0x20);
        memcpy(archive.getTree().get(), m_archive.getTree().get(), m_archive.getTreeSize());
        archive.setFiles(m_archive.getFiles());
        archive.setFilesSize(archive.getTree().get() - archive.getFiles());
        m_archive = archive;
        m_archiveSize = archiveSize;
        m_ownsMemory = true;
    }
    return true;
}

void ArchivePatcher::shrink() {
    if (!m_ownsMemory) {
        return;
    }

    Archive::Tree tree = m_archive.getTree();
    u8 *dst = m_archive.get() + 0x20;
    while (true) {
        u32 nodeIndex;
        u8 *src = nullptr;
        for (u32 i = 0; i < tree.getNodeCount(); i++) {
            Archive::Node node = tree.getNode(i);
            if (!node.isFile()) {
                continue;
            }
            if (node.getFileSize() == 0) {
                continue;
            }
            u8 *file = node.getFile(m_archive.getFiles());
            if (file >= dst && file < tree.get()) {
                if (!src || file < src) {
                    nodeIndex = i;
                    src = file;
                }
            }
        }
        if (!src) {
            break;
        }
        Archive::Node node = tree.getNode(nodeIndex);
        u32 size = AlignUp(node.getFileSize(), 0x20);
        memmove(dst, src, size);
        node.setFileOffset(dst - m_archive.getFiles());
        dst += size;
    }
    m_archive.setFilesSize(dst - m_archive.getFiles());
    tree = Archive::Tree(m_archive.getFiles() + m_archive.getFilesSize());
    u32 treeSize = m_archiveSize - m_archive.getTreeOffset();
    memmove(tree.get(), m_archive.getTree().get(), treeSize);
    m_archive.setTree(tree);
    u32 archiveSize = tree.get() + treeSize - m_archive.get();
    if (archiveSize != m_archiveSize) {
        if (m_heap->resize(m_archive.get(), archiveSize) >= 0) {
            DEBUG("Resized %s from 0x%lx to 0x%lx", m_bare, m_archiveSize, archiveSize);
            m_archiveSize = archiveSize;
        }
    }
}
