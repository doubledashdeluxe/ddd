#include "Bootstrap.hh"

#include <cube/Console.hh>
#include <cube/CubeLogger.hh>
#include <cube/ES.hh>
#include <cube/VI.hh>
#include <cube/ios/File.hh>
#include <cube/ios/KernelUID.hh>
#include <formats/Version.hh>
#include <portable/Algorithm.hh>
#include <portable/Array.hh>
#include <portable/Bytes.hh>
#include <portable/Log.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

extern "C" const u8 channel[];
extern "C" const size_t channel_size;

extern "C" u32 iosVersion;

void Bootstrap::Run() {
    VI::Init();
    Console::Init(VI::Instance());
    CubeLogger::Init();
    INFO("Double Dash Deluxe Bootstrap [%s]", Version);

    if (iosVersion >> 16 != 58 && iosVersion >> 16 != 59) {
        ERROR("Double Dash Deluxe needs IOS58 (or IOS59).");
        ERROR("Please perform a Wii System Update or use the IOS58 Installer to install");
        ERROR("IOS58.");
        return;
    }

    {
        INFO("Setting UID to kernel...");
        IOS::KernelUID kernelUID(false);
        if (!kernelUID.ok()) {
            ERROR("Failed to set UID to kernel.");
            return;
        }
        INFO("Set UID to kernel.");

        FS fs;
        if (!fs.ok()) {
            ERROR("Failed to initialize FS.");
            return;
        }

        if (!InstallChannelTicket(fs)) {
            ERROR("Failed to install the channel ticket.");
            return;
        }

        if (!InstallChannelContent(fs)) {
            ERROR("Failed to install the channel content.");
            return;
        }

        INFO("Setting UID back to PPC...");
    }
    INFO("Set UID back to PPC.");

    LaunchChannel();
    ERROR("Failed to launch the channel.");
}

bool Bootstrap::InstallChannelTicket(FS &fs) {
    alignas(0x20) Array<u8, 0x2a4> ticket(0x00);
    Bytes::WriteBE<u32>(ticket.values(), 0x000, 0x10001);            // RSA-2048 signature
    Bytes::WriteBE<u64>(ticket.values(), 0x1dc, 0x0001000844444443); // Title ID
    Bytes::WriteBE<u16>(ticket.values(), 0x1e4, 0xffff);             // Access mask
    memset(ticket.values() + 0x222, 0xff, 0x200 / 8);                // Content access mask

    if (IsFileInstalled("/ticket/00010008/44444443.tik", ticket.values(), ticket.count())) {
        INFO("An up-to-date channel ticket install was found.");
        return true;
    }

    INFO("Installing the channel ticket...");
    CreateDir(fs, "/ticket/00010008", 0660);
    if (!fs.writeFile("/tmp/44444443.tik", ticket.values(), ticket.count(), 0660)) {
        return false;
    }
    if (!fs.rename("/tmp/44444443.tik", "/ticket/00010008/44444443.tik")) {
        return false;
    }
    INFO("Installed the channel ticket.");
    return true;
}

bool Bootstrap::InstallChannelContent(FS &fs) {
    Array<Content, ContentCount> contents;
    contents[0].data = nullptr;
    contents[0].size = 0;
    contents[1].data = channel;
    contents[1].size = channel_size;
    alignas(0x20) Array<u8, 0x1e4 + ContentCount * 0x24> tmd(0x00);
    Bytes::WriteBE<u32>(tmd.values(), 0x000, 0x10001); // RSA-2048 signature
    Bytes::WriteBE<u64>(tmd.values(), 0x184, 0x0000000100000000 | iosVersion >> 16);
    Bytes::WriteBE<u64>(tmd.values(), 0x18c, 0x0001000844444443); // Title ID
    Bytes::WriteBE<u16>(tmd.values(), 0x19c, 3);                  // Region-free
    Bytes::WriteBE<u8>(tmd.values(), 0x1b2, 1);                   // Skip drive reset
    Bytes::WriteBE<u32>(tmd.values(), 0x1d8, 1 << 0);             // Full PPC hardware access
    Bytes::WriteBE<u16>(tmd.values(), 0x1de, ContentCount);
    Bytes::WriteBE<u16>(tmd.values(), 0x1e0, 1); // Boot index
    for (u32 i = 0; i < ContentCount; i++) {
        Bytes::WriteBE<u32>(tmd.values(), 0x1e4 + i * 0x24 + 0x00, i);      // Content ID
        Bytes::WriteBE<u16>(tmd.values(), 0x1e4 + i * 0x24 + 0x04, i);      // Content index
        Bytes::WriteBE<u16>(tmd.values(), 0x1e4 + i * 0x24 + 0x06, 0x0001); // Content type
        Bytes::WriteBE<u64>(tmd.values(), 0x1e4 + i * 0x24 + 0x08, contents[i].size);
    }

    if (IsFileInstalled("/title/00010008/44444443/content/title.tmd", tmd.values(), tmd.count())) {
        bool isInstalled = true;
        for (u32 i = 0; i < ContentCount; i++) {
            Array<char, 0x40> path;
            snprintf(path.values(), path.count(), "/title/00010008/44444443/content/%08lx.app", i);
            if (!IsFileInstalled(path.values(), contents[i].data, contents[i].size)) {
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
    CreateDir(fs, "/title/00010008", 0664);
    CreateDir(fs, "/title/00010008/44444443", 0664);
    CreateDir(fs, "/title/00010008/44444443/data", 0660);
    CreateDir(fs, "/tmp/content", 0660);
    if (!fs.writeFile("/tmp/content/title.tmd", tmd.values(), tmd.count(), 0660)) {
        return false;
    }
    for (u32 i = 0; i < ContentCount; i++) {
        Array<char, 0x40> path;
        snprintf(path.values(), path.count(), "/tmp/content/%08lx.app", i);
        if (!fs.writeFile(path.values(), contents[i].data, contents[i].size, 0660)) {
            return false;
        }
    }
    if (!fs.rename("/tmp/content", "/title/00010008/44444443/content")) {
        return false;
    }
    INFO("Installed the channel content.");
    return true;
}

void Bootstrap::LaunchChannel() {
    INFO("Launching the channel...");
    ES es;
    if (!es.ok()) {
        return;
    }
    Array<u8, 0xd8> ticketView(0x00);
    Bytes::WriteBE<u64>(ticketView.values(), 0x10, 0x0001000844444443); // Title ID
    Bytes::WriteBE<u16>(ticketView.values(), 0x18, 0xffff);             // Access mask
    memset(ticketView.values() + 0x56, 0xff, 0x200 / 8);                // Content access mask
    if (!es.launchTitle(0x0001000844444443, ticketView)) {
        return;
    }
    while (true) {}
}

bool Bootstrap::IsFileInstalled(const char *path, const void *data, u32 size) {
    IOS::File file(path, IOS::Mode::Read);
    if (!file.ok()) {
        return false;
    }

    IOS::File::Stats stats;
    if (!file.getStats(stats) || stats.size != size) {
        return false;
    }

    while (size > 0) {
        alignas(0x20) Array<u8, 0x1000> buffer;
        s32 chunkSize = Min<s32>(size, buffer.count());
        if (file.read(buffer.values(), chunkSize) != chunkSize) {
            return false;
        }
        if (memcmp(buffer.values(), data, chunkSize)) {
            return false;
        }
        data = static_cast<const u8 *>(data) + chunkSize;
        size -= chunkSize;
    }

    return true;
}

void Bootstrap::CreateDir(FS &fs, const char *path, u16 mode) {
    fs.createDir(path, mode);
    // Old DDD versions would create them with broken permissions
    fs.setAttr(path, mode);
}
