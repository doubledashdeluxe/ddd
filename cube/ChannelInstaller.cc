#include "ChannelInstaller.hh"

#include "cube/Platform.hh"
#include "cube/ios/File.hh"

#include <portable/Algorithm.hh>
#include <portable/Bytes.hh>
#include <portable/Log.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

bool ChannelInstaller::Install(const u8 *data, u32 size) {
    MemoryFile bootContent(data, size);
    return Install(bootContent);
}

bool ChannelInstaller::Install(Storage::FileHandle &file, u32 size, u32 offset) {
    StorageFile bootContent(file, size, offset);
    return Install(bootContent);
}

ChannelInstaller::File::File(u32 size) : m_size(size) {}

u32 ChannelInstaller::File::size() const {
    return m_size;
}

bool ChannelInstaller::File::isInstalled(const char *path) {
    IOS::File nandFile(path, IOS::Mode::Read);
    if (nandFile.ok()) {
        return false;
    }

    IOS::File::Stats stats;
    if (!nandFile.getStats(stats) || stats.size != m_size) {
        return false;
    }

    for (u32 offset = 0; offset < m_size;) {
        alignas(0x20) u8 chunk[MaxChunkSize];
        s32 chunkSize = Min<s32>(m_size - offset, Count(chunk));
        if (nandFile.read(chunk, chunkSize) != chunkSize) {
            return false;
        }
        if (!compare(chunk, chunkSize, offset)) {
            return false;
        }
        offset += chunkSize;
    }

    return true;
}

ChannelInstaller::MemoryFile::MemoryFile(const u8 *data, u32 size) : File(size), m_data(data) {}

bool ChannelInstaller::MemoryFile::install(FS &fs, const char *path) {
    return fs.writeFile(path, m_data, size(), 0660);
}

bool ChannelInstaller::MemoryFile::compare(const u8 (&data)[MaxChunkSize], u32 size, u32 offset) {
    return !memcmp(m_data + offset, data, size);
}

ChannelInstaller::StorageFile::StorageFile(Storage::FileHandle &file, u32 size, u32 offset)
    : File(size)
    , m_file(file)
    , m_offset(offset) {}

bool ChannelInstaller::StorageFile::install(FS &fs, const char *path) {
    fs.remove(path);
    fs.createFile(path, 0660, 0);

    IOS::File nandFile(path, IOS::Mode::Write);
    if (!nandFile.ok()) {
        return false;
    }

    for (u32 offset = 0; offset < size();) {
        alignas(0x20) u8 chunk[MaxChunkSize];
        s32 chunkSize = Min<s32>(size() - offset, Count(chunk));
        if (!m_file.read(chunk, chunkSize, m_offset + offset)) {
            return false;
        }
        if (nandFile.write(chunk, chunkSize) != chunkSize) {
            return false;
        }
        offset += chunkSize;
    }

    return true;
}

bool ChannelInstaller::StorageFile::compare(const u8 (&data)[MaxChunkSize], u32 size, u32 offset) {
    alignas(0x20) u8 buffer[MaxChunkSize];
    if (!m_file.read(buffer, size, m_offset + offset)) {
        return false;
    }
    return !memcmp(buffer, data, size);
}

ChannelInstaller::ChannelInstaller(File &bootContent) : m_bootContent(bootContent) {}

bool ChannelInstaller::install() {
    if (!m_fs.ok()) {
        ERROR("Failed to initialize FS.");
        return false;
    }

    if (!installTicket()) {
        ERROR("Failed to install the channel ticket.");
        return false;
    }

    if (!installContent()) {
        ERROR("Failed to install the channel content.");
        return false;
    }

    return true;
}

bool ChannelInstaller::installTicket() {
    alignas(0x20) u8 ticket[0x2a4] = {};
    Bytes::WriteBE<u32>(ticket, 0x000, 0x10001);            // RSA-2048 signature
    Bytes::WriteBE<u64>(ticket, 0x1dc, 0x0001000844444443); // Title ID
    Bytes::WriteBE<u16>(ticket, 0x1e4, 0xffff);             // Access mask
    memset(ticket + 0x222, 0xff, 0x200 / 8);                // Content access mask

    if (IsFileInstalled("/ticket/00010008/44444443.tik", ticket, sizeof(ticket))) {
        INFO("An up-to-date channel ticket install was found.");
        return true;
    }

    INFO("Installing the channel ticket...");
    createDir("/ticket/00010008", 0660);
    if (!installFile("/tmp/44444443.tik", ticket, sizeof(ticket))) {
        return false;
    }
    if (!m_fs.rename("/tmp/44444443.tik", "/ticket/00010008/44444443.tik")) {
        return false;
    }
    INFO("Installed the channel ticket.");
    return true;
}

bool ChannelInstaller::installContent() {
    MemoryFile dummyContent(nullptr, 0);
    File *contents[ContentCount] = {&dummyContent, &m_bootContent};
    alignas(0x20) u8 tmd[0x1e4 + ContentCount * 0x24] = {};
    Bytes::WriteBE<u32>(tmd, 0x000, 0x10001); // RSA-2048 signature
    Bytes::WriteBE<u64>(tmd, 0x184, 0x0000000100000000 | Platform::s_iosVersion >> 16);
    Bytes::WriteBE<u64>(tmd, 0x18c, 0x0001000844444443); // Title ID
    Bytes::WriteBE<u16>(tmd, 0x19c, 3);                  // Region-free
    Bytes::WriteBE<u8>(tmd, 0x1b2, 1);                   // Skip drive reset
    Bytes::WriteBE<u32>(tmd, 0x1d8, 1 << 0);             // Full PPC hardware access
    Bytes::WriteBE<u16>(tmd, 0x1de, ContentCount);
    Bytes::WriteBE<u16>(tmd, 0x1e0, 1); // Boot index
    for (u32 i = 0; i < ContentCount; i++) {
        Bytes::WriteBE<u32>(tmd, 0x1e4 + i * 0x24 + 0x00, i);      // Content ID
        Bytes::WriteBE<u16>(tmd, 0x1e4 + i * 0x24 + 0x04, i);      // Content index
        Bytes::WriteBE<u16>(tmd, 0x1e4 + i * 0x24 + 0x06, 0x0001); // Content type
        Bytes::WriteBE<u64>(tmd, 0x1e4 + i * 0x24 + 0x08, contents[i]->size());
    }

    if (IsFileInstalled("/title/00010008/44444443/content/title.tmd", tmd, sizeof(tmd))) {
        bool isInstalled = true;
        for (u32 i = 0; i < ContentCount; i++) {
            char path[0x40];
            snprintf(path, Count(path), "/title/00010008/44444443/content/%08lx.app", i);
            if (!contents[i]->isInstalled(path)) {
                isInstalled = false;
                break;
            }
        }
        if (isInstalled) {
            INFO("An up-to-date channel content install was found.");
            return true;
        }
    }

    INFO("Installing the channel content...");
    createDir("/title/00010008", 0664);
    createDir("/title/00010008/44444443", 0664);
    createDir("/title/00010008/44444443/data", 0660);
    createDir("/tmp/content", 0660);
    if (!installFile("/tmp/content/title.tmd", tmd, Count(tmd))) {
        return false;
    }
    for (u32 i = 0; i < ContentCount; i++) {
        char path[0x40];
        snprintf(path, Count(path), "/tmp/content/%08lx.app", i);
        if (!contents[i]->install(m_fs, path)) {
            return false;
        }
    }
    if (!m_fs.rename("/tmp/content", "/title/00010008/44444443/content")) {
        return false;
    }
    INFO("Installed the channel content.");
    return true;
}

void ChannelInstaller::createDir(const char *path, u16 mode) {
    m_fs.createDir(path, mode);
    // Old DDD versions would create them with broken permissions
    m_fs.setAttr(path, mode);
}

bool ChannelInstaller::installFile(const char *path, const u8 *data, u32 size) {
    MemoryFile file(data, size);
    return file.install(m_fs, path);
}

bool ChannelInstaller::Install(File &bootContent) {
    ChannelInstaller installer(bootContent);
    return installer.install();
}

bool ChannelInstaller::IsFileInstalled(const char *path, const u8 *data, u32 size) {
    MemoryFile file(data, size);
    return file.isInstalled(path);
}
