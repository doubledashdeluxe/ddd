#pragma once

#include "common/Array.hh"

#ifdef PAYLOAD
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <payload/Mutex.hh>
#endif

class Storage {
public:
    class File;
    class Dir;

private:
    class StorageHandle;

public:
    class NodeType {
    public:
        enum {
            File,
            Dir,
        };

    private:
        NodeType();
    };

    struct NodeInfo {
        u32 type;
        Array<char, 256> name;
    };

    class Mode {
    public:
        enum {
            Read,
            WriteAlways,
            WriteNew,
            RemoveAlways,
            RemoveExisting,
        };

    private:
        Mode();
    };

    class FileHandle {
    public:
        FileHandle();
        FileHandle(const char *path, u32 mode);
        ~FileHandle();

        void open(const char *path, u32 mode);
        void close();
        bool read(void *dst, u32 size, u32 offset);
        bool write(const void *src, u32 size, u32 offset);
        bool sync();
        bool size(u64 &size);

    private:
        FileHandle(const FileHandle &);
        FileHandle &operator=(const FileHandle &);

        File *m_file;

        friend class StorageHandle;
    };

    class DirHandle {
    public:
        DirHandle();
        DirHandle(const char *path);
        ~DirHandle();

        void open(const char *path);
        void close();
        bool read(NodeInfo &nodeInfo);

    private:
        DirHandle(const DirHandle &);
        DirHandle &operator=(const DirHandle &);

        Dir *m_dir;

        friend class StorageHandle;
    };

    class File {
    protected:
        File();

        virtual void close();
        virtual bool read(void *dst, u32 size, u32 offset) = 0;
        virtual bool write(const void *src, u32 size, u32 offset) = 0;
        virtual bool sync() = 0;
        virtual bool size(u64 &size) = 0;
        virtual Storage *storage() = 0;

    private:
        FileHandle *m_handle;

        friend class FileHandle;
        friend class StorageHandle;
    };

    class Dir {
    protected:
        Dir();

        virtual void close();
        virtual bool read(NodeInfo &nodeInfo) = 0;
        virtual Storage *storage() = 0;

    private:
        DirHandle *m_handle;

        friend class DirHandle;
        friend class StorageHandle;
    };

    static void Init();

    static bool ReadFile(const char *path, void *dst, u32 size, u32 *readSize);
    static bool WriteFile(const char *path, const void *src, u32 size, u32 mode);

    static bool CreateDir(const char *path, u32 mode);

    static bool Rename(const char *srcPath, const char *dstPath);
    static bool Remove(const char *path, u32 mode);

protected:
    Storage();

    void notify();
    void remove();
    void add();

    virtual void poll() = 0;
    virtual u32 priority() = 0;
    virtual const char *prefix() = 0;

    virtual File *openFile(const char *path, u32 mode) = 0;

    virtual Dir *openDir(const char *path) = 0;
    virtual bool createDir(const char *path, u32 mode) = 0;

    virtual bool rename(const char *srcPath, const char *dstPath) = 0;
    virtual bool remove(const char *path, u32 mode) = 0;

private:
    class StorageHandle {
    public:
        StorageHandle(const char *path);
        StorageHandle(const FileHandle &file);
        StorageHandle(const DirHandle &dir);
        ~StorageHandle();

        const char *prefix();

        File *openFile(const char *path, u32 mode);

        Dir *openDir(const char *path);
        bool createDir(const char *path, u32 mode);

        bool rename(const char *srcPath, const char *dstPath);
        bool remove(const char *path, u32 mode);

    private:
        StorageHandle(const StorageHandle &);
        StorageHandle &operator=(const StorageHandle &);

        Storage *m_storage;
        const char *m_prefix;
    };

    ~Storage();

#ifdef PAYLOAD
    static void *Poll(void *param);
#endif

    Storage *m_next;
#ifdef PAYLOAD
    Mutex m_mutex;
#else
    u8 _08[0x20 - 0x08];
#endif

    static Storage *s_head;
#ifdef PAYLOAD
    static OSMessageQueue s_queue;
    static Array<OSMessage, 1> s_messages;
#endif
};
static_assert(sizeof(Storage) == 0x20);
