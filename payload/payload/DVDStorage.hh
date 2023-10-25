#pragma once

#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/DVD.h>
}

class DVDStorage : private Storage {
public:
    static void Init();

private:
    class File : public Storage::File {
    public:
        File();

    private:
        void close();
        bool read(void *dst, u32 size, u32 offset);
        bool write(const void *src, u32 size, u32 offset);
        bool sync();
        bool size(u64 &size);
        Storage *storage();

        DVDStorage *m_storage;
        DVDFileInfo m_fileInfo;

        friend class DVDStorage;
    };

    class Dir : public Storage::Dir {
    public:
        Dir();

    private:
        void close();
        bool read(NodeInfo &nodeInfo);
        Storage *storage();

        DVDStorage *m_storage;
        DVDDir m_dir;

        friend class DVDStorage;
    };

    DVDStorage();

    void poll();
    u32 priority();
    const char *prefix();

    Storage::File *openFile(const char *path, u32 mode);

    Storage::Dir *openDir(const char *path);
    bool createDir(const char *path, u32 mode);

    bool rename(const char *srcPath, const char *dstPath);
    bool remove(const char *path, u32 mode);

    template <typename N>
    static N *FindNode(Array<N, 32> &nodes) {
        for (u32 i = 0; i < nodes.count(); i++) {
            if (!nodes[i].m_storage) {
                return &nodes[i];
            }
        }

        return nullptr;
    }

    Array<File, 32> m_files;
    Array<Dir, 32> m_dirs;

    static DVDStorage *s_instance;
};
