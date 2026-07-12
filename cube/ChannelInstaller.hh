#pragma once

#include "cube/FS.hh"
#include "cube/storage/Storage.hh"

class ChannelInstaller {
public:
    static bool Install(const u8 *data, u32 size);
    static bool Install(Storage::FileHandle &file, u32 size, u32 offset);

private:
    enum {
        MaxChunkSize = 4 * 1024,
    };

    class File {
    public:
        File(u32 size);
        u32 size() const;
        bool isInstalled(const char *path);
        virtual bool install(FS &fs, const char *path) = 0;

    private:
        virtual bool compare(const u8 (&data)[MaxChunkSize], u32 size, u32 offset) = 0;

        u32 m_size;
    };

    class MemoryFile : public File {
    public:
        MemoryFile(const u8 *data, u32 size);
        bool install(FS &fs, const char *path) override;

    private:
        bool compare(const u8 (&data)[MaxChunkSize], u32 size, u32 offset) override;

        const u8 *m_data;
    };

    class StorageFile : public File {
    public:
        StorageFile(Storage::FileHandle &file, u32 size, u32 offset);
        bool install(FS &fs, const char *path) override;

    private:
        bool compare(const u8 (&data)[MaxChunkSize], u32 size, u32 offset) override;

        Storage::FileHandle &m_file;
        u32 m_offset;
    };

    ChannelInstaller(File &bootContent);
    bool install();
    bool installTicket();
    bool installContent();
    void createDir(const char *path, u16 mode);
    bool installFile(const char *path, const u8 *data, u32 size);

    static bool Install(File &bootContent);
    static bool IsFileInstalled(const char *path, const u8 *data, u32 size);

    File &m_bootContent;
    FS m_fs;

    static const u32 ContentCount = 2;
};
