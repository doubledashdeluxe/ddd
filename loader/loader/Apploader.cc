#include "Apploader.hh"

#include "loader/Clock.hh"
#include "loader/DI.hh"

#include <common/Align.hh>
#include <common/Console.hh>
#include <common/ICache.hh>
#include <common/Log.hh>

extern "C" {
#include <string.h>
}

extern "C" u32 discID[8];

Apploader::GameEntryFunc Apploader::LoadAndRun() {
    if (!DI::ReadDiscID()) {
        return nullptr;
    }

    char discIDString[5] = {0};
    memcpy(discIDString, &discID[0], sizeof(discID[0]));

    if (!IsDiscIDValid()) {
        ERROR("This is not Mario Kart: Double Dash!! (disc id %s).\n", discIDString);

        while (DI::IsInserted()) {
            Clock::WaitMilliseconds(100);
        }
        return nullptr;
    }
    INFO("Mario Kart: Double Dash!! disc found (disc id %s).\n", discIDString);

    alignas(0x20) ApploaderHeader header;
    if (!DI::Read(&header, sizeof(header), 0x2440)) {
        ERROR("Failed to read apploader header.\n");
        return nullptr;
    }
    INFO("Successfully read apploader header.\n");

    if (!DI::Read(reinterpret_cast<void *>(0x81200000), AlignUp(header.size + header.trailer, 0x20),
                0x2460)) {
        ERROR("Failed to read apploader.\n");
        return nullptr;
    }
    ICache::Invalidate(reinterpret_cast<void *>(0x81200000), header.size + header.trailer);
    INFO("Successfully read apploader.\n");

    ApploaderInitFunc init;
    ApploaderMainFunc main;
    ApploaderCloseFunc close;
    header.entry(&init, &main, &close);

    init(Report);

    void *dst;
    u32 size;
    u32 offset;
    while (main(&dst, &size, &offset)) {
        if (!DI::Read(dst, size, offset)) {
            ERROR("Failed to read dol section.\n");
            return nullptr;
        }
        ICache::Invalidate(dst, size);
    }
    INFO("Successfully read dol.\n");

    return close();
}

bool Apploader::IsDiscIDValid() {
    if ((discID[0] >> 8) != (('G' << 16) | ('M' << 8) | '4')) {
        return false;
    }

    switch (discID[0] & 0xff) {
    case 'P':
    case 'E':
    case 'J':
        return true;
    default:
        return false;
    }
}

void Apploader::Report(const char * /* format */, ...) {}
