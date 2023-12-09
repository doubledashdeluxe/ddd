#pragma once

#include "payload/Archive.hh"

#include <common/storage/Storage.hh>
#include <jsystem/JKRHeap.hh>

class FileArchive {
public:
    class Node;

    class Dir : public Archive::Dir {
    public:
        Dir(FileArchive &archive, const char *path, bool &ok);

        char *getName() const;
        Node getNode(u32 index) const;

    private:
        Dir(Archive::Dir dir, FileArchive &archive);

        FileArchive &m_archive;

        friend class Node;
    };

    class Node : public Archive::Node {
    public:
        Node(FileArchive &archive, const char *path, bool &ok);

        char *getName() const;
        Dir getDir() const;
        bool getFile(void *file) const;
        void *getFile(JKRHeap *heap, s32 alignment) const;

    private:
        Node(Archive::Node node, FileArchive &archive);

        FileArchive &m_archive;

        friend class Dir;
    };

    FileArchive(JKRHeap *heap, s32 alignment, const char *path, bool &ok);
    ~FileArchive();

private:
    JKRHeap *m_heap;
    Storage::FileHandle m_file;
    Archive m_archive;
    Archive::Tree m_tree;
};
