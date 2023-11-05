#include "JKRMemArchive.hh"

#include <common/Align.hh>
#include <common/Bytes.hh>
#include <common/Log.hh>
#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/DVD.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>
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
    s32 length = snprintf(filePath.values(), filePath.count(), "dvd:%s", path.values());
    if (length < 0 || static_cast<size_t>(length) >= filePath.count()) {
        return false;
    }

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

    if (!parseTree(treeSize)) {
        m_heap->free(m_archive);
        return false;
    }

    m_mountMode = MountMode::Mem;
    m_ownsMemory = true;

    if (strncmp(path.values(), "/Course/", strlen("/Course/"))) {
        const char *bare = strrchr(path.values(), '/');
        bare = bare ? bare + 1 : path.values();
        Array<char, 256> relative('\0');
        addSubdir(fileSize, bare, relative);
    }

    return true;
}

bool JKRMemArchive::open(void *archive, u32 /* r5 */, u32 memBreakFlag) {
    m_archive = archive;
    u8 *header = reinterpret_cast<u8 *>(m_archive);
    u32 treeOffset = Bytes::ReadBE<u32>(header, 0x08);
    u32 treeSize = Bytes::ReadBE<u32>(header, 0x0c);
    u32 filesOffset = treeOffset + treeSize;

    m_tree = header + treeOffset;
    m_files = header + filesOffset;

    if (!parseTree(treeSize)) {
        return false;
    }

    m_heap = JKRHeap::FindFromRoot(this);
    m_mountMode = MountMode::Mem;
    m_ownsMemory = memBreakFlag == MemBreakFlag::Break;

    u64 archiveSize = 0;
    u8 *dir = m_dirs;
    Array<char, 256> bare;
    snprintf(bare.values(), bare.count(), "%s.arc", m_names + Bytes::ReadBE<u16>(dir, 0x06));
    Array<char, 256> relative('\0');
    addSubdir(archiveSize, bare.values(), relative);

    return true;
}

bool JKRMemArchive::parseTree(u32 treeSize) {
    u32 dirsOffset = Bytes::ReadBE<u32>(m_tree, 0x04);
    u32 dirsSize = Bytes::ReadBE<u32>(m_tree, 0x00) * 0x10;
    if (dirsOffset > treeSize || dirsSize > treeSize || dirsOffset + dirsSize > treeSize) {
        return false;
    }
    m_dirs = m_tree + dirsOffset;

    u32 nodesOffset = Bytes::ReadBE<u32>(m_tree, 0x0c);
    u32 nodesSize = Bytes::ReadBE<u32>(m_tree, 0x08) * 0x14;
    if (nodesOffset > treeSize || nodesSize > treeSize || nodesOffset + nodesSize > treeSize) {
        return false;
    }
    m_nodes = m_tree + nodesOffset;

    u32 namesOffset = Bytes::ReadBE<u32>(m_tree, 0x14);
    u32 namesSize = Bytes::ReadBE<u32>(m_tree, 0x10);
    if (namesOffset > treeSize || namesSize > treeSize || namesOffset + namesSize > treeSize) {
        return false;
    }
    m_names = reinterpret_cast<char *>(m_tree + namesOffset);

    return true;
}

bool JKRMemArchive::addSubdir(u64 &archiveSize, const char *bare, Array<char, 256> &relative) {
    u64 oldArchiveSize = archiveSize;

    const char *name;
    u8 *dir, *node = m_nodes;
    bool exists = true;
    if (relative[0] != '\0') {
        if (!searchNode(relative, name, dir, node, exists)) {
            return false;
        }
    }

    if (exists) {
        bool isDir = Bytes::ReadBE<u8>(node, 0x04) & 0x2;
        if (!isDir) {
            return false;
        }
    } else {
        if (!relocate(archiveSize, dir, node)) {
            return false;
        }

        u32 dirCount = Bytes::ReadBE<u32>(m_tree, 0x00);
        u32 nodeCount = Bytes::ReadBE<u32>(m_tree, 0x08);

        if (!addNode(archiveSize, name, dir, node)) {
            return false;
        }
        Bytes::WriteBE<u16>(node, 0x00, UINT16_MAX);
        Bytes::WriteBE<u8>(node, 0x04, 0x02);
        Bytes::WriteBE<u32>(node, 0x08, dirCount);
        Bytes::WriteBE<u32>(node, 0x0c, 0x10);

        dir = m_dirs + dirCount * 0x10;
        if (!allocAt(archiveSize, archiveSize + 0x10, dir, dir, node)) {
            return false;
        }
        dir = m_dirs + dirCount * 0x10;
        for (u32 i = 0; i < 4; i++) {
            if (i < strlen(name)) {
                dir[i] = toupper(name[i]);
            } else {
                dir[i] = '\0';
            }
        }
        Bytes::WriteBE<u32>(dir, 0x04, Bytes::ReadBE<u16>(node, 0x06));
        Bytes::WriteBE<u16>(dir, 0x08, Bytes::ReadBE<u16>(node, 0x02));
        Bytes::WriteBE<u16>(dir, 0x0a, 0);
        Bytes::WriteBE<u32>(dir, 0x0c, nodeCount);

        Bytes::WriteBE<u32>(m_tree, 0x00, dirCount + 1);

        DEBUG("Added %s to %s (%llx)\n", relative.values(), bare, archiveSize - oldArchiveSize);
    }

    Array<char, 256> dirPath;
    s32 length = snprintf(dirPath.values(), dirPath.count(), "main:/ddd/assets/%s%s", bare,
            relative.values());
    if (length < 0 || static_cast<size_t>(length) >= dirPath.count()) {
        return false;
    }

    length = strlen(relative.values());
    Storage::NodeInfo nodeInfo;
    for (Storage::DirHandle dir(dirPath.values()); dir.read(nodeInfo);) {
        snprintf(relative.values() + length, relative.count() - length, "/%s",
                nodeInfo.name.values());
        if (nodeInfo.type == Storage::NodeType::Dir) {
            if (!addSubdir(archiveSize, bare, relative)) {
                return false;
            }
        } else {
            if (!addSubfile(archiveSize, bare, relative)) {
                return false;
            }
        }
    }

    return true;
}

bool JKRMemArchive::addSubfile(u64 &archiveSize, const char *bare,
        const Array<char, 256> &relative) {
    u64 oldArchiveSize = archiveSize;

    const char *name;
    u8 *dir, *node;
    bool exists;
    if (!searchNode(relative, name, dir, node, exists)) {
        return false;
    }

    if (!relocate(archiveSize, dir, node)) {
        return false;
    }

    if (exists) {
        bool isDir = Bytes::ReadBE<u8>(node, 0x04) & 0x2;
        if (isDir) {
            return false;
        }
    } else {
        u16 fileCount = Bytes::ReadBE<u16>(m_tree, 0x18);

        if (!addNode(archiveSize, name, dir, node)) {
            return false;
        }
        Bytes::WriteBE<u16>(node, 0x00, fileCount);
        Bytes::WriteBE<u8>(node, 0x04, 0x11);
        Bytes::WriteBE<u32>(node, 0x08, m_tree - reinterpret_cast<u8 *>(m_archive));
        Bytes::WriteBE<u32>(node, 0x0c, 0);

        Bytes::WriteBE<u16>(m_tree, 0x18, fileCount + 1);
    }

    Array<char, 256> subfilePath;
    s32 length = snprintf(subfilePath.values(), subfilePath.count(), "main:/ddd/assets/%s%s", bare,
            relative.values());
    if (length < 0 || static_cast<size_t>(length) >= subfilePath.count()) {
        return false;
    }

    Storage::FileHandle subfile(subfilePath.values(), Storage::Mode::Read);
    u64 subfileSize;
    if (!subfile.size(subfileSize)) {
        return false;
    }
    u64 alignedSubfileSize = AlignUp(subfileSize, 0x20);
    if (alignedSubfileSize > UINT32_MAX) {
        return false;
    }

    u32 oldSubfileSize = Bytes::ReadBE<u32>(node, 0x0c);
    u8 *file;
    if (alignedSubfileSize > oldSubfileSize) {
        if (!allocAt(archiveSize, archiveSize + alignedSubfileSize, m_tree, dir, node)) {
            return false;
        }
        file = m_tree - alignedSubfileSize;
    } else {
        if (!allocAt(archiveSize, archiveSize + oldSubfileSize,
                    reinterpret_cast<u8 *>(m_archive) + archiveSize, dir, node)) {
            return false;
        }
        file = m_files + Bytes::ReadBE<u32>(node, 0x08);
        memcpy(reinterpret_cast<u8 *>(m_archive) + archiveSize - oldSubfileSize, file,
                oldSubfileSize);
    }

    if (!subfile.read(file, subfileSize, 0x0)) {
        if (alignedSubfileSize > oldSubfileSize) {
            allocAt(archiveSize, archiveSize - alignedSubfileSize, m_tree, dir, node);
        } else {
            memcpy(file, reinterpret_cast<u8 *>(m_archive) + archiveSize, oldSubfileSize);
            allocAt(archiveSize, archiveSize - oldSubfileSize,
                    reinterpret_cast<u8 *>(m_archive) + archiveSize, dir, node);
        }
        return false;
    }
    if (alignedSubfileSize <= oldSubfileSize) {
        if (!allocAt(archiveSize, archiveSize - oldSubfileSize,
                    reinterpret_cast<u8 *>(m_archive) + archiveSize, dir, node)) {
            return false;
        }
    }

    Bytes::WriteBE<u32>(node, 0x08, file - m_files);
    Bytes::WriteBE<u32>(node, 0x0c, subfileSize);

    DEBUG("Added %s to %s (%llx)\n", relative.values(), bare, archiveSize - oldArchiveSize);
    return true;
}

bool JKRMemArchive::searchNode(const Array<char, 256> &relative, const char *&name, u8 *&dir,
        u8 *&node, bool &exists) {
    dir = m_dirs;
    name = relative.values() + 1;
    for (const char *nextName; *name; name = nextName) {
        nextName = strchr(name, '/');
        u32 nameLength = nextName ? nextName - name : strlen(name);
        nextName = nextName ? nextName + 1 : name + nameLength;

        u16 nodeCount = Bytes::ReadBE<u16>(dir, 0x0a);
        u16 firstNodeIndex = Bytes::ReadBE<u32>(dir, 0x0c);
        u16 nodeIndex;
        for (nodeIndex = firstNodeIndex; nodeIndex < firstNodeIndex + nodeCount; nodeIndex++) {
            node = m_nodes + nodeIndex * 0x14;
            u16 nameOffset = Bytes::ReadBE<u16>(node, 0x06);
            if (!strncmp(m_names + nameOffset, name, nameLength)) {
                break;
            }
        }
        if (nodeIndex == firstNodeIndex + nodeCount) {
            exists = false;
            node = m_nodes + nodeIndex * 0x14;
            return !*nextName;
        }

        if (*nextName) {
            bool isDir = Bytes::ReadBE<u8>(node, 0x04) & 0x2;
            if (isDir) {
                dir = m_dirs + Bytes::ReadBE<u32>(node, 0x08) * 0x10;
            } else {
                return false;
            }
        }
    }
    exists = true;
    return true;
}

bool JKRMemArchive::addNode(u64 &archiveSize, const char *name, u8 *&dir, u8 *&node) {
    u32 nameLength = strlen(name);
    u32 nameSize = AlignUp(nameLength + 1, 0x4);
    u16 nameHash = 0;
    for (u32 i = 0; i < nameLength; i++) {
        nameHash *= 3;
        nameHash += name[i];
    }

    u32 dirCount = Bytes::ReadBE<u32>(m_tree, 0x00);
    u32 nodeCount = Bytes::ReadBE<u32>(m_tree, 0x08);
    u32 namesSize = Bytes::ReadBE<u32>(m_tree, 0x10);

    u32 dirIndex = (dir - m_dirs) / 0x10;
    u32 nodeIndex = (node - m_nodes) / 0x14;
    if (!allocAt(archiveSize, archiveSize + 0x14, node, dir, node)) {
        return false;
    }
    dir = m_dirs + dirIndex * 0x10;
    node = m_nodes + nodeIndex * 0x14;

    Bytes::WriteBE<u16>(node, 0x02, nameHash);
    Bytes::WriteBE<u8>(node, 0x05, 0x00);
    Bytes::WriteBE<u16>(node, 0x06, namesSize);
    Bytes::WriteBE<u32>(node, 0x10, 0x0);

    for (u32 dirIndex = 0; dirIndex < dirCount; dirIndex++) {
        if (m_dirs + dirIndex * 0x10 == dir) {
            continue;
        }
        u16 firstNodeIndex = Bytes::ReadBE<u32>(m_dirs + dirIndex * 0x10, 0x0c);
        if (firstNodeIndex >= nodeIndex) {
            Bytes::WriteBE<u32>(m_dirs + dirIndex * 0x10, 0x0c, firstNodeIndex + 1);
        }
    }

    if (!allocAt(archiveSize, archiveSize + nameSize, m_names + namesSize, dir, node)) {
        return false;
    }
    snprintf(m_names + namesSize, nameLength + 1, "%s", name);

    Bytes::WriteBE<u16>(dir, 0x0a, Bytes::ReadBE<u16>(dir, 0x0a) + 1);

    Bytes::WriteBE<u32>(m_tree, 0x08, nodeCount + 1);
    Bytes::WriteBE<u32>(m_tree, 0x10, namesSize + nameSize);

    return true;
}

bool JKRMemArchive::relocate(u64 &archiveSize, u8 *&dir, u8 *&node) {
    if (m_ownsMemory) {
        return true;
    }

    u8 *header = reinterpret_cast<u8 *>(m_archive);
    u64 newArchiveSize = Bytes::ReadBE<u32>(header, 0x04);
    s32 align = m_mountDirection == MountDirection::Head ? 0x20 : -0x20;
    void *newArchive = m_heap->alloc(newArchiveSize, align);
    if (!newArchive) {
        return false;
    }

    u8 *newHeader = reinterpret_cast<u8 *>(newArchive);
    memcpy(newHeader, header, 0x20);

    u32 treeOffset = Bytes::ReadBE<u32>(newHeader, 0x08);
    u32 treeSize = Bytes::ReadBE<u32>(newHeader, 0x0c);
    if (treeOffset > newArchiveSize || treeSize > newArchiveSize ||
            treeOffset + treeSize > newArchiveSize) {
        m_heap->free(newArchive);
        return false;
    }
    u32 filesOffset = treeOffset + treeSize;
    u32 filesSize = newArchiveSize - filesOffset;
    u8 *newTree = newHeader + 0x20 + filesSize;
    memcpy(newTree, m_tree, treeSize);
    u8 *newFiles = newHeader + 0x20;
    memcpy(newFiles, m_files, filesSize);

    archiveSize = newArchiveSize;
    m_archive = newArchive;
    m_dirs += newTree - m_tree;
    m_nodes += newTree - m_tree;
    m_names += newTree - m_tree;
    dir += newTree - m_tree;
    node += newTree - m_tree;
    m_tree = newTree;
    m_files = newFiles;
    m_ownsMemory = true;
    return true;
}

bool JKRMemArchive::allocAt(u64 &archiveSize, u64 newArchiveSize, void *ptr, u8 *&dir, u8 *&node) {
    if (newArchiveSize > UINT32_MAX) {
        return false;
    }

    s32 offset = newArchiveSize - archiveSize;
    if (offset < 0) {
        move(archiveSize, ptr, offset, dir, node);
    }

    if (m_heap->resize(m_archive, newArchiveSize) < 0) {
        s32 align = m_mountDirection == MountDirection::Head ? 0x20 : -0x20;
        void *newArchive = m_heap->alloc(newArchiveSize, align);
        if (!newArchive) {
            return false;
        }
        s32 offset = reinterpret_cast<u8 *>(newArchive) - reinterpret_cast<u8 *>(m_archive);
        memcpy(newArchive, m_archive, archiveSize);
        m_heap->free(m_archive);
        m_archive = newArchive;
        m_tree += offset;
        m_files += offset;
        m_dirs += offset;
        m_nodes += offset;
        m_names += offset;
        ptr = reinterpret_cast<u8 *>(ptr) + offset;
        node += offset;
        dir += offset;
    }

    if (offset > 0) {
        move(archiveSize, ptr, offset, dir, node);
    }

    archiveSize = newArchiveSize;
    return true;
}

void JKRMemArchive::move(u64 archiveSize, void *ptr, s32 offset, u8 *&dir, u8 *&node) {
    memmove(reinterpret_cast<u8 *>(ptr) + offset, ptr,
            archiveSize - (reinterpret_cast<u8 *>(ptr) - reinterpret_cast<u8 *>(m_archive)));
    m_tree = m_tree >= ptr ? m_tree + offset : m_tree;
    m_files = m_files >= ptr ? m_files + offset : m_files;
    m_dirs = m_dirs >= ptr ? m_dirs + offset : m_dirs;
    m_nodes = m_nodes >= ptr ? m_nodes + offset : m_nodes;
    m_names = m_names >= ptr ? m_names + offset : m_names;
    node = node >= ptr ? node + offset : node;
    dir = dir >= ptr ? dir + offset : dir;
}
