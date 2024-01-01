#pragma once

#include "common/Array.hh"

class Mutex;
extern "C" {
struct OSMessageQueue;
}

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
            ReadWrite,
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

        bool open(const char *path, u32 mode);
        void close();
        bool read(void *dst, u32 size, u32 offset);
        bool write(const void *src, u32 size, u32 offset);
        bool sync();
        bool truncate(u64 size);
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

        bool open(const char *path);
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
        virtual bool truncate(u64 size) = 0;
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

    class Observer {
    public:
        Observer();
        Observer *next();

        virtual void onAdd(const char *prefix) = 0;
        virtual void onRemove(const char *prefix) = 0;

    private:
        Observer *m_next;
    };

    static void Init();

    static bool ReadFile(const char *path, void *dst, u32 size, u32 *readSize);
    static bool WriteFile(const char *path, const void *src, u32 size, u32 mode);

    static bool CreateDir(const char *path, u32 mode);

    static bool Rename(const char *srcPath, const char *dstPath);
    static bool Remove(const char *path, u32 mode);

protected:
    Storage(Mutex *mutex);
    ~Storage();

    bool isContained() const;
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

        void acquireWithoutLocking(const char *path);
        void acquireWithoutLocking(const FileHandle &file);
        void acquireWithoutLocking(const DirHandle &dir);

        Storage *m_storage;
        const char *m_prefix;
    };

    void removeWithoutLocking();
    void addWithoutLocking();

    static void *Poll(void *param);

    Storage *m_next;
    bool m_isContained;
    Mutex *m_mutex;

    static Storage *s_head;
    static Observer *s_headObserver;
    static OSMessageQueue s_queue;
};
