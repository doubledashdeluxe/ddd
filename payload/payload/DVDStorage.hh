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
        void close() override;
        bool read(void *dst, u32 size, u32 offset) override;
        bool write(const void *src, u32 size, u32 offset) override;
        bool sync() override;
        bool size(u64 &size) override;
        Storage *storage() override;

        DVDStorage *m_storage;
        DVDFileInfo m_fileInfo;

        friend class DVDStorage;
    };

    class Dir : public Storage::Dir {
    public:
        Dir();

    private:
        void close() override;
        bool read(NodeInfo &nodeInfo) override;
        Storage *storage() override;

        DVDStorage *m_storage;
        DVDDir m_dir;

        friend class DVDStorage;
    };

    DVDStorage();

    void poll() override;
    u32 priority() override;
    const char *prefix() override;

    Storage::File *openFile(const char *path, u32 mode) override;

    Storage::Dir *openDir(const char *path) override;
    bool createDir(const char *path, u32 mode) override;

    bool rename(const char *srcPath, const char *dstPath) override;
    bool remove(const char *path, u32 mode) override;

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
    alignas(0x20) Array<u8, 0x10000> m_buffer;
    OSMessageQueue m_initQueue;
    Array<OSMessage, 1> m_initMessages;

    static DVDStorage *s_instance;
};
