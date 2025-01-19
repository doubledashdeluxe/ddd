#pragma once

#include <common/storage/Storage.hh>

#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class FakeStorage : private Storage {
public:
    FakeStorage();
    ~FakeStorage();

private:
    class File : public Storage::File {
    public:
        File(FakeStorage *storage, std::string path);
        ~File();

    private:
        void close() override;
        bool read(void *dst, u32 size, u32 offset) override;
        bool write(const void *src, u32 size, u32 offset) override;
        bool sync() override;
        bool truncate(u64 size) override;
        bool size(u64 &size) override;
        Storage *storage() override;

        FakeStorage *m_storage;
        std::string m_path;
    };

    class Dir : public Storage::Dir {
    public:
        Dir(FakeStorage *storage, std::string path);
        ~Dir();

    private:
        void close() override;
        bool read(NodeInfo &nodeInfo) override;
        Storage *storage() override;

        FakeStorage *m_storage;
        std::string m_path;
        size_t m_index = 0;
    };

    void poll() override;
    u32 priority() override;
    const char *prefix() override;

    Storage::File *openFile(const char *path, u32 mode) override;

    Storage::Dir *openDir(const char *path) override;
    bool createDir(const char *path, u32 mode) override;

    bool rename(const char *srcPath, const char *dstPath) override;
    bool remove(const char *path, u32 mode) override;

    bool hasFile(const std::string &path);
    bool hasDir(const std::string &path);
    bool hasParentDir(const std::string &path);

    std::unordered_map<std::string, std::vector<u8>> m_files;
    std::unordered_set<std::string> m_dirs;
    std::unordered_map<std::string, std::unique_ptr<File>> m_openFiles;
    std::unordered_map<std::string, std::unique_ptr<Dir>> m_openDirs;
};
