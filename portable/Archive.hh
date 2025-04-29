#pragma once

#include "portable/Types.hh"

class Archive {
public:
    class Dir;
    class Node;

    class Tree {
    public:
        Tree(u8 *tree);
        ~Tree();

        bool isValid(u32 treeSize, u32 filesSize) const;
        bool isHeaderValid(u32 treeSize) const;
        u8 *get() const;
        Dir getDir(u32 index) const;
        u32 getDirCount() const;
        Node getNode(u32 index) const;
        u32 getNodeCount() const;
        char *getNames() const;
        u32 getNamesSize() const;
        u16 getFileCount() const;
        bool search(const char *path, const char *&name, Dir &dir, Node &node, bool &exists) const;

        void setDirs(u8 *dirs) const;
        void setDirCount(u32 dirCount) const;
        void setNodes(u8 *nodes) const;
        void setNodeCount(u32 nodeCount) const;
        void setNames(char *names) const;
        void setNamesSize(u32 namesSize) const;
        void setFileCount(u16 fileCount) const;

    private:
        u8 *m_tree;
    };

    class Dir {
    public:
        Dir(u8 *dir);
        ~Dir();

        bool isValid(Tree tree) const;
        u8 *get() const;
        u32 getType() const;
        u32 getNameOffset() const;
        char *getName(char *names) const;
        u16 getNameHash() const;
        u16 getNodeCount() const;
        u32 getFirstNodeIndex() const;
        Node getNode(u32 index, Tree tree) const;

        void setType(u32 type) const;
        void setNameOffset(u32 nameOffset) const;
        void setNameHash(u32 nameHash) const;
        void setNodeCount(u32 nodeCount) const;
        void setFirstNodeIndex(u32 firstNodeIndex) const;

    private:
        u8 *m_dir;
    };

    class Node {
    public:
        Node(u8 *node);
        ~Node();

        bool isValid(Tree tree, u32 filesSize) const;
        u8 *get() const;
        u16 getFileIndex() const;
        u16 getNameHash() const;
        bool isDir() const;
        bool isFile() const;
        u32 getNameOffset() const;
        char *getName(char *names) const;
        u32 getDirIndex() const;
        Dir getDir(Tree tree) const;
        u32 getFileOffset() const;
        u8 *getFile(u8 *files) const;
        u32 getFileSize() const;
        u32 getFileAddress() const;

        void setFileIndex(u16 fileIndex) const;
        void setNameHash(u16 nameHash) const;
        void setType(u8 type) const;
        void setNameOffset(u32 nameOffset) const;
        void setDirIndex(u32 dirIndex) const;
        void setFileOffset(u32 fileOffset) const;
        void setFileSize(u32 fileSize) const;
        void setFileAddress(u32 fileAddress) const;

    private:
        u8 *m_node;
    };

    Archive(u8 *archive);
    ~Archive();

    bool isValid(u32 archiveSize) const;
    bool isHeaderValid(u32 archiveSize) const;
    u8 *get() const;
    u32 getArchiveSize() const;
    u32 getTreeOffset() const;
    Tree getTree() const;
    u32 getTreeSize() const;
    u32 getFilesOffset() const;
    u8 *getFiles() const;
    u32 getFilesSize() const;

    void setArchiveSize(u32 archiveSize) const;
    void setTreeOffset(u32 treeOffset) const;
    void setTree(Tree tree) const;
    void setTreeSize(u32 treeSize) const;
    void setFilesOffset(u32 filesOffset) const;
    void setFiles(u8 *files) const;
    void setFilesSize(u32 filesSize) const;

private:
    u8 *m_archive;
};
