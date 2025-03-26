#pragma once

#include <common/FS.hh>

class Bootstrap {
public:
    static void Run();

private:
    enum {
        ContentCount = 2,
    };

    struct Content {
        const void *data;
        u32 size;
    };

    Bootstrap();

    static bool InstallChannelTicket(FS &fs);
    static bool InstallChannelContent(FS &fs);
    static void LaunchChannel();

    static bool IsFileInstalled(const char *path, const void *data, u32 size);
    static void CreateDir(FS &fs, const char *path, u16 mode);
};
