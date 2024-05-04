#include "Loader.hh"

#include "loader/Apploader.hh"
#include "loader/DI.hh"

#include <common/Arena.hh>
#include <common/Clock.hh>
#include <common/DCache.hh>
#include <common/DiscID.hh>
#include <common/ICache.hh>
#include <common/Log.hh>
#include <common/Platform.hh>
#include <common/SC.hh>
#include <common/USB.hh>
#include <common/VirtualDI.hh>
#include <common/storage/SDStorage.hh>
#include <common/storage/Storage.hh>
#include <common/storage/USBStorage.hh>

extern "C" {
#include <string.h>
}

extern "C" const u8 payloadP[];
extern "C" const size_t payloadP_size;
extern "C" const u8 payloadE[];
extern "C" const size_t payloadE_size;
extern "C" const u8 payloadJ[];
extern "C" const size_t payloadJ_size;

extern "C" const u8 commonArchive[];
extern "C" const size_t commonArchive_size;

extern "C" const u8 japaneseArchive[];
extern "C" const size_t japaneseArchive_size;
extern "C" const u8 englishArchive[];
extern "C" const size_t englishArchive_size;
extern "C" const u8 germanArchive[];
extern "C" const size_t germanArchive_size;
extern "C" const u8 frenchArchive[];
extern "C" const size_t frenchArchive_size;
extern "C" const u8 spanishArchive[];
extern "C" const size_t spanishArchive_size;
extern "C" const u8 italianArchive[];
extern "C" const size_t italianArchive_size;

extern "C" u32 consoleType;
extern "C" u32 arenaLo;
extern "C" u32 iosVersion;

extern "C" volatile u32 aicr;

extern "C" volatile u32 armirqmask;
extern "C" volatile u32 aipprot;

Loader::PayloadEntryFunc Loader::Run(Context *context) {
    // Use compat MMIO ranges for DI/SI/EXI/AI
    aipprot &= ~0x1;

    // Prevent Starlet from receiving DI interrupts
    armirqmask &= ~(1 << 18);

    // Reset the DSP: libogc apps like the HBC cannot initialize it properly, but the SDK can.
    aicr = 0;

    VI::Init();

    Console::Init(VI::Instance());
    INFO("Double Dash Deluxe Loader");

    if (iosVersion >> 16 != 58 && iosVersion >> 16 != 59) {
        ERROR("In order for Double Dash Deluxe to work, IOS58 (or IOS59) must be installed.");
        ERROR("Please perform a Wii System Update or use the IOS58 Installer to install IOS58.");
        return nullptr;
    }

    if (Platform::IsDolphin()) {
        Array<char, 64> dolphinVersion;
        if (!Platform::GetDolphinVersion(dolphinVersion) ||
                strncmp(dolphinVersion.values(), "5.0-20189", strlen("5.0-20189")) < 0) {
            WARN("Using DSP HLE will result in broken sound on this Dolphin version!");
            WARN("Please update to Dolphin 5.0-20189 or later.");
            Clock::WaitMilliseconds(1000);
        }
    }

    Storage::Init();
    USBStorage::Init();
    USB::Init();
    SDStorage::Init();
    VirtualDI::Init();

    RunApploader(context);

    void *payloadDst;
    const void *payloadSrc;
    size_t payloadSize;
    switch (DiscID::Get().gameID[3]) {
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
        ERROR("Region detection failed!");
        return nullptr;
    }

    INFO("Copying payload...");
    memcpy(payloadDst, payloadSrc, payloadSize);
    DCache::Flush(payloadDst, payloadSize);
    ICache::Invalidate(payloadDst, payloadSize);
    INFO("Copied payload.");

    INFO("Copying common archive...");
    context->commonArchive = MEM2Arena::Instance()->alloc(commonArchive_size, 0x20);
    context->commonArchiveSize = commonArchive_size;
    memcpy(context->commonArchive, &commonArchive, commonArchive_size);
    INFO("Copied common archive.");

    INFO("Copying localized archive...");
    u32 language = GetLanguage();
    const u8 *localizedArchives[] = {
            japaneseArchive,
            englishArchive,
            germanArchive,
            frenchArchive,
            spanishArchive,
            italianArchive,
    };
    size_t localizedArchiveSizes[] = {
            japaneseArchive_size,
            englishArchive_size,
            germanArchive_size,
            frenchArchive_size,
            spanishArchive_size,
            italianArchive_size,
    };
    const u8 *localizedArchive = localizedArchives[language];
    size_t localizedArchiveSize = localizedArchiveSizes[language];
    context->localizedArchive = MEM2Arena::Instance()->alloc(localizedArchiveSize, 0x20);
    context->localizedArchiveSize = localizedArchiveSize;
    memcpy(context->localizedArchive, localizedArchive, localizedArchiveSize);
    INFO("Copied localized archive.");

    if (Platform::IsDolphin()) {
        // Enable OSReport over EXI
        consoleType = 0x10000006;
    }
    arenaLo = reinterpret_cast<uintptr_t>(payloadDst) + payloadSize;

    INFO("Starting payload...");
    PayloadEntryFunc payloadEntry = reinterpret_cast<PayloadEntryFunc>(payloadDst);
    return payloadEntry;
}

void Loader::RunApploader(Context *context) {
    while (true) {
        if (DI::ReadDiscID()) {
            const char *gameID = DiscID::Get().gameID;
            if (DiscID::IsValid()) {
                INFO("Mario Kart: Double Dash!! disc found (game id %.4s).", gameID);
                if (Apploader::Run(DI::Read)) {
                    context->hasVirtualDI = false;
                    return;
                }
            } else {
                ERROR("This is not Mario Kart: Double Dash!! (game id %.4s).", gameID);
            }
            while (DI::ReadDiscID()) {
                Clock::WaitMilliseconds(100);
                if (RunApploaderFromVirtualDI()) {
                    context->hasVirtualDI = true;
                    return;
                }
            }
        } else {
            if (DI::IsInserted()) {
                INFO("Resetting disc interface...");
                DI::Reset();
                INFO("Reset disc interface.");
            } else {
                if (Platform::IsDolphin()) {
                    WARN("Insert the Mario Kart: Double Dash!! disc by right-clicking the game");
                    WARN("in the game list and selecting \"Change Disc\".");
                    WARN("To avoid this in the future, select \"Set as Default ISO\" as well.");
                } else {
                    WARN("Please insert a Mario Kart: Double Dash!! disc.");
                }
                while (!DI::IsInserted()) {
                    Clock::WaitMilliseconds(100);
                    if (RunApploaderFromVirtualDI()) {
                        context->hasVirtualDI = true;
                        return;
                    }
                }
            }
        }
    }
}

bool Loader::RunApploaderFromVirtualDI() {
    return RunApploaderFromVirtualDI(!Platform::IsDolphin());
}

bool Loader::RunApploaderFromVirtualDI(bool enableUSB) {
    if (enableUSB) {
        USB::Handle usbHandle;

        return RunApploaderFromVirtualDI(false);
    }

    SDStorage sdStorage;

    if (!VirtualDI::Mount()) {
        return false;
    }

    if (!Apploader::Run(VirtualDI::Read)) {
        return false;
    }

    return true;
}

u32 Loader::GetLanguage() {
    char region = DiscID::Get().gameID[3];
    if (region == 'J') {
        return Language::Japanese;
    }

    if (region == 'P') {
        u8 language;
        SC sc;
        if (sc.get("IPL.LNG", language)) {
            switch (language) {
            case Language::English:
            case Language::German:
            case Language::French:
            case Language::Spanish:
            case Language::Italian:
                return language;
            }
        }
    }

    return Language::English;
}
