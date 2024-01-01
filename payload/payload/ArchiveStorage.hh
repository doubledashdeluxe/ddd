#pragma once

#include "payload/Archive.hh"
#include "payload/Mutex.hh"

#include <common/storage/Storage.hh>
extern "C" {
#include <dolphin/OSMessage.h>
}

class ArchiveStorage : private Storage {
public:
    static void Init(const char *prefix, void *archive, u32 archiveSize);

private:
    class File : public Storage::File {
    public:
        File();

    private:
        void close() override;
        bool read(void *dst, u32 size, u32 offset) override;
        bool write(const void *src, u32 size, u32 offset) override;
        bool sync() override;
        bool truncate(u64 size) override;
        bool size(u64 &size) override;
        Storage *storage() override;

        ArchiveStorage *m_storage;
        Archive::Node m_node;

        friend class ArchiveStorage;
    };

    class Dir : public Storage::Dir {
    public:
        Dir();

    private:
        void close() override;
        bool read(NodeInfo &nodeInfo) override;
        Storage *storage() override;

        ArchiveStorage *m_storage;
        Archive::Dir m_dir;
        u16 m_index;

        friend class ArchiveStorage;
    };

    ArchiveStorage(const char *prefix, void *archive, u32 archiveSize);

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

    Mutex m_mutex;
    ArchiveStorage *m_next;
    const char *m_prefix;
    Archive m_archive;
    u32 m_archiveSize;
    Array<File, 32> m_files;
    Array<Dir, 32> m_dirs;
    OSMessageQueue m_initQueue;
    Array<OSMessage, 1> m_initMessages;

    static ArchiveStorage *s_head;
};
