#include "Loader.hh"

#include "loader/Clock.hh"
#include "loader/DI.hh"

#include <common/Console.hh>
#include <common/DCache.hh>
#include <common/ICache.hh>
#include <common/Log.hh>
#include <common/Platform.hh>
#include <common/VI.hh>

extern "C" {
#include <string.h>
}

extern "C" const u8 payloadP[];
extern "C" const size_t payloadP_size;
extern "C" const u8 payloadE[];
extern "C" const size_t payloadE_size;
extern "C" const u8 payloadJ[];
extern "C" const size_t payloadJ_size;

extern "C" u32 discID[8];
extern "C" u32 consoleType;
extern "C" u32 arenaLo;
extern "C" u32 iosVersion;

extern "C" volatile u32 aicr;

extern "C" volatile u32 armirqmask;
extern "C" volatile u32 aipprot;

Apploader::GameEntryFunc Loader::Run() {
    // Use compat MMIO ranges for DI/SI/EXI/AI
    aipprot &= ~0x1;

    // Prevent Starlet from receiving DI interrupts
    armirqmask &= ~(1 << 18);

    // Reset the DSP: libogc apps like the HBC cannot initialize it properly, but the SDK can.
    aicr = 0;

    VI::Init();

    Console::Init();
    INFO("Double Dash Deluxe Loader\n");
    INFO("\n");

    if (iosVersion >> 16 != 58 && iosVersion >> 16 != 59) {
        ERROR("In order for Double Dash Deluxe to work, IOS58 (or IOS59) must be installed.\n");
        ERROR("Please perform a Wii System Update or use the IOS58 Installer to install IOS58.\n");
        return nullptr;
    }

    bool diWasReset = false;
    Apploader::GameEntryFunc gameEntry;
    while (!(gameEntry = Apploader::LoadAndRun())) {
        if (!DI::IsInserted()) {
            if (Platform::IsDolphin()) {
                if (diWasReset) {
                    INFO("\n");
                }
                INFO("Insert the Mario Kart: Double Dash!! disc by right-clicking the game\n");
                INFO("in the game list and select \"Change Disc\".\n");
                INFO("\n");
                INFO("To avoid this in the future, select \"Set as Default ISO\" as well.\n");
                INFO("\n");
            } else {
                INFO("Please insert a Mario Kart: Double Dash!! disc.\n");
            }

            while (!DI::IsInserted()) {
                Clock::WaitMilliseconds(100);
            }
        }

        Clock::WaitMilliseconds(100);

        INFO("Resetting disc interface...");
        DI::Reset();
        INFO(" done.\n");
        diWasReset = true;
    }

    void *payloadDst;
    const void *payloadSrc;
    size_t payloadSize;
    switch (discID[0] & 0xff) {
    case 'P':
        payloadDst = reinterpret_cast<void *>(0x803ec140);
        payloadSrc = &payloadP;
        payloadSize = payloadP_size;
        break;
    case 'E':
        payloadDst = reinterpret_cast<void *>(0x803e2300);
        payloadSrc = &payloadE;
        payloadSize = payloadE_size;
        break;
    case 'J':
        payloadDst = reinterpret_cast<void *>(0x803fc920);
        payloadSrc = &payloadJ;
        payloadSize = payloadJ_size;
        break;
    default:
        ERROR("Region detection failed!\n");
        return nullptr;
    }

    INFO("Copying payload...");
    memcpy(payloadDst, payloadSrc, payloadSize);
    DCache::Flush(payloadDst, payloadSize);
    ICache::Invalidate(payloadDst, payloadSize);
    INFO(" done.\n");

    INFO("Applying patches...");
    PayloadEntryFunc payloadEntry = reinterpret_cast<PayloadEntryFunc>(payloadDst);
    payloadEntry();
    INFO(" done.\n");

    if (Platform::IsDolphin()) {
        // Enable OSReport over EXI
        consoleType = 0x10000006;
    }
    arenaLo = reinterpret_cast<u32>(payloadDst) + payloadSize;

    return gameEntry;
}
