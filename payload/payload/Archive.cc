#include "Archive.hh"

#include <common/Align.hh>
#include <common/Bytes.hh>
#include <common/Memory.hh>

extern "C" {
#include <string.h>
#include <strings.h>
}

Archive::Tree::Tree(u8 *tree) : m_tree(tree) {}

Archive::Tree::~Tree() {}

bool Archive::Tree::isValid(u32 treeSize, u32 filesSize) const {
    if (!Memory::IsAligned(m_tree, 0x04)) {
        return false;
    }

    if (!isHeaderValid(treeSize)) {
        return false;
    }

    for (u32 i = 0; i < getDirCount(); i++) {
        if (!getDir(i).isValid(*this)) {
            return false;
        }
    }

    for (u32 i = 0; i < getNodeCount(); i++) {
        if (!getNode(i).isValid(*this, filesSize)) {
            return false;
        }
    }

    return true;
}

bool Archive::Tree::isHeaderValid(u32 treeSize) const {
    if (treeSize < 0x20) {
        return false;
    }

    u32 dirsOffset = Bytes::ReadBE<u32>(m_tree, 0x04);
    u32 dirsSize = Bytes::ReadBE<u32>(m_tree, 0x00) * 0x10;
    if (!IsAligned(dirsOffset, 0x4) || !IsAligned(dirsSize, 0x4)) {
        return false;
    }
    if (dirsOffset > treeSize || dirsSize > treeSize) {
        return false;
    }
    if (dirsOffset + dirsSize < dirsOffset || dirsOffset + dirsSize > treeSize) {
        return false;
    }
    if (dirsSize == 0) {
        return false;
    }

    u32 nodesOffset = Bytes::ReadBE<u32>(m_tree, 0x0c);
    u32 nodesSize = Bytes::ReadBE<u32>(m_tree, 0x08) * 0x14;
    if (!IsAligned(nodesOffset, 0x4) || !IsAligned(nodesSize, 0x4)) {
        return false;
    }
    if (nodesOffset > treeSize || nodesSize > treeSize) {
        return false;
    }
    if (nodesOffset + nodesSize < nodesOffset || nodesOffset + nodesSize > treeSize) {
        return false;
    }
    if (nodesSize == 0) {
        return false;
    }

    u32 namesOffset = Bytes::ReadBE<u32>(m_tree, 0x14);
    u32 namesSize = Bytes::ReadBE<u32>(m_tree, 0x10);
    if (!IsAligned(namesOffset, 0x4) || !IsAligned(namesSize, 0x4)) {
        return false;
    }
    if (namesOffset > treeSize || namesSize > treeSize) {
        return false;
    }
    if (namesOffset + nodesSize < namesOffset || namesOffset + namesSize > treeSize) {
        return false;
    }

    if (nodesOffset < dirsOffset || namesOffset < nodesOffset) {
        return false;
    }
    if (dirsOffset + dirsSize > nodesOffset || nodesOffset + nodesSize > namesOffset) {
        return false;
    }

    return true;
}

u8 *Archive::Tree::get() const {
    return m_tree;
}

Archive::Dir Archive::Tree::getDir(u32 index) const {
    return Dir(m_tree + Bytes::ReadBE<u32>(m_tree, 0x04) + index * 0x10);
}

u32 Archive::Tree::getDirCount() const {
    return Bytes::ReadBE<u32>(m_tree, 0x00);
}

Archive::Node Archive::Tree::getNode(u32 index) const {
    return Node(m_tree + Bytes::ReadBE<u32>(m_tree, 0x0c) + index * 0x14);
}

u32 Archive::Tree::getNodeCount() const {
    return Bytes::ReadBE<u32>(m_tree, 0x08);
}

char *Archive::Tree::getNames() const {
    return reinterpret_cast<char *>(m_tree + Bytes::ReadBE<u32>(m_tree, 0x14));
}

u32 Archive::Tree::getNamesSize() const {
    return Bytes::ReadBE<u32>(m_tree, 0x10);
}

u16 Archive::Tree::getFileCount() const {
    return Bytes::ReadBE<u16>(m_tree, 0x18);
}

bool Archive::Tree::search(const char *path, const char *&name, Dir &dir, Node &node,
        bool &exists) const {
    dir = getDir(0);
    name = path + 1;
    for (const char *nextName; *name; name = nextName) {
        nextName = strchr(name, '/');
        u32 nameLength = nextName ? nextName - name : strlen(name);
        nextName = nextName ? nextName + 1 : name + nameLength;

        u32 i;
        for (i = 0; i < dir.getNodeCount(); i++) {
            node = dir.getNode(i, *this);
            if (!strncasecmp(node.getName(getNames()), name, nameLength)) {
                break;
            }
        }
        if (i == dir.getNodeCount()) {
            exists = false;
            node = dir.getNode(dir.getNodeCount(), *this);
            return !*nextName;
        }

        if (node.isDir()) {
            dir = node.getDir(*this);
        } else {
            if (*nextName) {
                return false;
            }
        }
    }
    exists = true;
    return true;
}

void Archive::Tree::setDirs(u8 *dirs) const {
    Bytes::WriteBE<u32>(m_tree, 0x04, dirs - m_tree);
}

void Archive::Tree::setDirCount(u32 dirCount) const {
    Bytes::WriteBE<u32>(m_tree, 0x00, dirCount);
}

void Archive::Tree::setNodes(u8 *nodes) const {
    Bytes::WriteBE<u32>(m_tree, 0x0c, nodes - m_tree);
}

void Archive::Tree::setNodeCount(u32 nodeCount) const {
    Bytes::WriteBE<u32>(m_tree, 0x08, nodeCount);
}

void Archive::Tree::setNames(char *names) const {
    Bytes::WriteBE<u32>(m_tree, 0x14, reinterpret_cast<u8 *>(names) - m_tree);
}

void Archive::Tree::setNamesSize(u32 namesSize) const {
    Bytes::WriteBE<u32>(m_tree, 0x10, namesSize);
}

void Archive::Tree::setFileCount(u16 fileCount) const {
    Bytes::WriteBE<u16>(m_tree, 0x18, fileCount);
}

Archive::Dir::Dir(u8 *dir) : m_dir(dir) {}

Archive::Dir::~Dir() {}

bool Archive::Dir::isValid(Tree tree) const {
    if (!Memory::IsAligned(m_dir, 0x04)) {
        return false;
    }

    u32 nameOffset = Bytes::ReadBE<u32>(m_dir, 0x04);
    if (nameOffset >= tree.getNamesSize()) {
        return false;
    }
    const char *n = tree.getNames() + nameOffset;
    for (u32 o = nameOffset; o < tree.getNamesSize(); o++, n++) {
        if (*n == '\0') {
            break;
        }
    }
    if (*n != '\0') {
        return false;
    }

    u32 firstNode = Bytes::ReadBE<u32>(m_dir, 0x0c);
    u16 nodeCount = getNodeCount();
    if (firstNode > tree.getNodeCount() || nodeCount > tree.getNodeCount()) {
        return false;
    }
    if (firstNode + nodeCount < firstNode || firstNode + nodeCount > tree.getNodeCount()) {
        return false;
    }

    return true;
}

u8 *Archive::Dir::get() const {
    return m_dir;
}

u32 Archive::Dir::getType() const {
    return Bytes::ReadBE<u32>(m_dir, 0x00);
}

u32 Archive::Dir::getNameOffset() const {
    return Bytes::ReadBE<u32>(m_dir, 0x04);
}

char *Archive::Dir::getName(char *names) const {
    return names + getNameOffset();
}

u16 Archive::Dir::getNameHash() const {
    return Bytes::ReadBE<u16>(m_dir, 0x08);
}

u16 Archive::Dir::getNodeCount() const {
    return Bytes::ReadBE<u16>(m_dir, 0x0a);
}

u32 Archive::Dir::getFirstNodeIndex() const {
    return Bytes::ReadBE<u32>(m_dir, 0x0c);
}

Archive::Node Archive::Dir::getNode(u32 index, Tree tree) const {
    return tree.getNode(getFirstNodeIndex() + index);
}

void Archive::Dir::setType(u32 type) const {
    Bytes::WriteBE<u32>(m_dir, 0x00, type);
}

void Archive::Dir::setNameOffset(u32 nameOffset) const {
    Bytes::WriteBE<u32>(m_dir, 0x04, nameOffset);
}

void Archive::Dir::setNameHash(u32 nameHash) const {
    Bytes::WriteBE<u16>(m_dir, 0x08, nameHash);
}

void Archive::Dir::setNodeCount(u32 nodeCount) const {
    Bytes::WriteBE<u16>(m_dir, 0x0a, nodeCount);
}

void Archive::Dir::setFirstNodeIndex(u32 firstNodeIndex) const {
    Bytes::WriteBE<u32>(m_dir, 0x0c, firstNodeIndex);
}

Archive::Node::Node(u8 *node) : m_node(node) {}

Archive::Node::~Node() {}

bool Archive::Node::isValid(Tree tree, u32 filesSize) const {
    if (!Memory::IsAligned(m_node, 0x04)) {
        return false;
    }

    if (isDir() && isFile()) {
        return false;
    }

    u32 nameOffset = Bytes::ReadBE<u32>(m_node, 0x04) & 0xffffff;
    if (nameOffset >= tree.getNamesSize()) {
        return false;
    }
    const char *n = tree.getNames() + nameOffset;
    for (u32 o = nameOffset; o < tree.getNamesSize(); o++, n++) {
        if (*n == '\0') {
            break;
        }
    }
    if (*n != '\0') {
        return false;
    }

    if (isDir()) {
        if (Bytes::ReadBE<u32>(m_node, 0x08) >= tree.getDirCount()) {
            return false;
        }
    }

    if (isFile()) {
        u32 fileOffset = Bytes::ReadBE<u32>(m_node, 0x08);
        u32 fileSize = getFileSize();
        if (!IsAligned(fileOffset, 0x20)) {
            return false;
        }
        if (fileOffset > filesSize || fileSize > filesSize) {
            return false;
        }
        if (fileOffset + fileSize < fileOffset || fileOffset + fileSize > filesSize) {
            return false;
        }
    }

    return true;
}

u8 *Archive::Node::get() const {
    return m_node;
}

u16 Archive::Node::getFileIndex() const {
    return Bytes::ReadBE<u16>(m_node, 0x00);
}

u16 Archive::Node::getNameHash() const {
    return Bytes::ReadBE<u16>(m_node, 0x02);
}

bool Archive::Node::isDir() const {
    return Bytes::ReadBE<u8>(m_node, 0x04) & 0x2 && Bytes::ReadBE<u32>(m_node, 0x08) != 0xffffffff;
}

bool Archive::Node::isFile() const {
    return Bytes::ReadBE<u8>(m_node, 0x04) & 0x1;
}

u32 Archive::Node::getNameOffset() const {
    return Bytes::ReadBE<u32>(m_node, 0x04) & 0xffffff;
}

char *Archive::Node::getName(char *names) const {
    return reinterpret_cast<char *>(names + getNameOffset());
}

u32 Archive::Node::getDirIndex() const {
    return Bytes::ReadBE<u32>(m_node, 0x08);
}

Archive::Dir Archive::Node::getDir(Tree tree) const {
    return tree.getDir(getDirIndex());
}

u32 Archive::Node::getFileOffset() const {
    return Bytes::ReadBE<u32>(m_node, 0x08);
}

u8 *Archive::Node::getFile(u8 *files) const {
    return files + getFileOffset();
}

u32 Archive::Node::getFileSize() const {
    return Bytes::ReadBE<u32>(m_node, 0x0c);
}

u32 Archive::Node::getFileAddress() const {
    return Bytes::ReadBE<u32>(m_node, 0x10);
}

void Archive::Node::setFileIndex(u16 fileIndex) const {
    Bytes::WriteBE<u16>(m_node, 0x00, fileIndex);
}

void Archive::Node::setNameHash(u16 nameHash) const {
    Bytes::WriteBE<u16>(m_node, 0x02, nameHash);
}

void Archive::Node::setType(u8 type) const {
    Bytes::WriteBE<u8>(m_node, 0x04, type);
}

void Archive::Node::setNameOffset(u32 nameOffset) const {
    u8 type = Bytes::ReadBE<u8>(m_node, 0x04);
    Bytes::WriteBE<u32>(m_node, 0x04, nameOffset | type << 24);
}

void Archive::Node::setDirIndex(u32 dirIndex) const {
    Bytes::WriteBE<u32>(m_node, 0x08, dirIndex);
}

void Archive::Node::setFileOffset(u32 fileOffset) const {
    Bytes::WriteBE<u32>(m_node, 0x08, fileOffset);
}

void Archive::Node::setFileSize(u32 fileSize) const {
    Bytes::WriteBE<u32>(m_node, 0x0c, fileSize);
}

void Archive::Node::setFileAddress(u32 fileAddress) const {
    Bytes::WriteBE<u32>(m_node, 0x10, fileAddress);
}

Archive::Archive(u8 *archive) : m_archive(archive) {}

Archive::~Archive() {}

bool Archive::isValid(u32 archiveSize) const {
    if (!Memory::IsAligned(m_archive, 0x20)) {
        return false;
    }

    if (!isHeaderValid(archiveSize)) {
        return false;
    }

    if (!getTree().isValid(getTreeSize(), getFilesSize())) {
        return false;
    }

    return true;
}

bool Archive::isHeaderValid(u32 archiveSize) const {
    if (archiveSize < 0x20) {
        return false;
    }

    if (memcmp(m_archive, "RARC", 0x04)) {
        return false;
    }

    if (Bytes::ReadBE<u32>(m_archive, 0x04) != archiveSize) {
        return false;
    }

    u32 treeOffset = Bytes::ReadBE<u32>(m_archive, 0x08);
    u32 treeSize = Bytes::ReadBE<u32>(m_archive, 0x0c);
    if (!IsAligned(treeOffset, 0x20) || !IsAligned(treeSize, 0x20)) {
        return false;
    }
    if (treeOffset > archiveSize || treeSize > archiveSize) {
        return false;
    }
    if (treeOffset + treeSize < treeOffset || treeOffset + treeSize > archiveSize) {
        return false;
    }

    if (Bytes::ReadBE<u32>(m_archive, 0x10) != archiveSize - (treeOffset + treeSize)) {
        return false;
    }
    if (Bytes::ReadBE<u32>(m_archive, 0x14) != archiveSize - (treeOffset + treeSize)) {
        return false;
    }

    return true;
}

u8 *Archive::get() const {
    return m_archive;
}

u32 Archive::getTreeOffset() const {
    return Bytes::ReadBE<u32>(m_archive, 0x08);
}

Archive::Tree Archive::getTree() const {
    return Tree(m_archive + getTreeOffset());
}

u32 Archive::getTreeSize() const {
    return Bytes::ReadBE<u32>(m_archive, 0x0c);
}

u32 Archive::getFilesOffset() const {
    return Bytes::ReadBE<u32>(m_archive, 0x08) + Bytes::ReadBE<u32>(m_archive, 0x0c);
}

u8 *Archive::getFiles() const {
    return m_archive + getFilesOffset();
}

u32 Archive::getFilesSize() const {
    return Bytes::ReadBE<u32>(m_archive, 0x10);
}

void Archive::setTreeOffset(u32 treeOffset) const {
    u32 filesOffset = getFilesOffset();
    Bytes::WriteBE<u32>(m_archive, 0x08, treeOffset);
    setFilesOffset(filesOffset);
}

void Archive::setTree(Tree tree) const {
    setTreeOffset(tree.get() - m_archive);
}

void Archive::setTreeSize(u32 treeSize) const {
    Bytes::WriteBE<u32>(m_archive, 0x0c, treeSize);
}

void Archive::setFilesOffset(u32 filesOffset) const {
    setTreeSize(filesOffset - getTreeOffset());
}

void Archive::setFiles(u8 *files) const {
    setFilesOffset(files - m_archive);
}

void Archive::setFilesSize(u32 filesSize) const {
    Bytes::WriteBE<u32>(m_archive, 0x10, filesSize);
}
