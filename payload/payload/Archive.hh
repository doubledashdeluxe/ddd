#pragma once

#include <common/Types.hh>

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
        bool search(const char *path, const char *&name, Dir &dir, Node &node, bool &exists) const;

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
        char *getName(char *names) const;
        u16 getNameHash() const;
        u16 getNodeCount() const;
        Node getNode(u32 index, Tree tree) const;

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
        char *getName(char *names) const;
        Dir getDir(Tree tree) const;
        u32 getFileOffset() const;
        u8 *getFile(u8 *files) const;
        u32 getFileSize() const;

    private:
        u8 *m_node;
    };

    Archive(u8 *archive);
    ~Archive();

    bool isValid(u32 archiveSize) const;
    bool isHeaderValid(u32 archiveSize) const;
    u8 *get() const;
    u32 getTreeOffset() const;
    Tree getTree() const;
    u32 getTreeSize() const;
    u32 getFilesOffset() const;
    u8 *getFiles() const;
    u32 getFilesSize() const;

private:
    u8 *m_archive;
};
