#include "Channel.hh"

#include "channel/Apploader.hh"
#include "channel/DI.hh"
#include "channel/SRAM.hh"

#include <cube/Arena.hh>
#include <cube/Clock.hh>
#include <cube/Console.hh>
#include <cube/CubeLogger.hh>
#include <cube/DCache.hh>
#include <cube/DiscID.hh>
#include <cube/ICache.hh>
#include <cube/Platform.hh>
#include <cube/USB.hh>
#include <cube/VI.hh>
#include <cube/VirtualDI.hh>
#include <cube/ios/Resource.hh>
#include <cube/storage/EXISDStorage.hh>
#include <cube/storage/Storage.hh>
#include <cube/storage/USBStorage.hh>
#include <cube/storage/WiiSDStorage.hh>
#include <formats/Version.hh>
extern "C" {
#include <miniz/miniz.h>
}
#include <portable/Align.hh>
#include <portable/Log.hh>

extern "C" {
#include <string.h>
}

extern "C" const u8 payloadPI[];
extern "C" const size_t payloadPI_size;
extern "C" const u8 payloadEI[];
extern "C" const size_t payloadEI_size;
extern "C" const u8 payloadJI[];
extern "C" const size_t payloadJI_size;

extern "C" const u8 payloadPD[];
extern "C" const size_t payloadPD_size;
extern "C" const u8 payloadED[];
extern "C" const size_t payloadED_size;
extern "C" const u8 payloadJD[];
extern "C" const size_t payloadJD_size;

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
extern "C" u32 arenaHi;
extern "C" u32 iosVersion;

extern "C" volatile u32 aicr;

extern "C" volatile u32 armirqmask;
extern "C" volatile u32 aipprot;

Channel::PayloadEntryFunc Channel::Run(Context *context) {
    VI::Init();
    Console::Init(VI::Instance());
    CubeLogger::Init();
    INFO("Double Dash Deluxe Channel v%u.%u.%u", MajorVersion, MinorVersion, PatchVersion);

    if (!Platform::IsGameCube()) {
        if (iosVersion >> 16 != 58 && iosVersion >> 16 != 59) {
            ERROR("Double Dash Deluxe needs IOS58 (or IOS59).");
            ERROR("Please perform a Wii System Update or use the IOS58 Installer to install");
            ERROR("IOS58.");
            return nullptr;
        }

        IOS::Resource::Init();
    }
    Clock::Init();

    if (Platform::IsDolphin()) {
        DolphinVersion dolphinVersion;
        DolphinVersion minimumDolphinVersion = {5, 0, 20189};
        if (!Platform::GetDolphinVersion(dolphinVersion) ||
                dolphinVersion < minimumDolphinVersion) {
            WARN("Using DSP HLE will result in broken sound on this Dolphin version!");
            WARN("Please update to Dolphin 5.0-20189 or later.");
            Clock::WaitMilliseconds(1000);
        }
    }

    if (!Platform::IsGameCube()) {
        // Use compat MMIO ranges for DI/SI/EXI/AI
        aipprot &= ~(1 << 0);

        // Prevent Starlet from receiving DI interrupts
        armirqmask &= ~(1 << 18);
    }

    // Reset the DSP: libogc apps like the HBC cannot initialize it properly, but the SDK can.
    aicr = 0;

    Storage::Init();
    if (!Platform::IsGameCube()) {
        USBStorage::Init();
        USB::Init();
        WiiSDStorage::Init();
    }
    VirtualDI::Init();

    RunApploader(context);

    void *payloadIDst;
    const void *payloadISrc;
    size_t payloadISrcSize;
    const void *payloadDSrc;
    size_t payloadDSrcSize;
    switch (DiscID::Get().gameID[3]) {
    case 'P':
        payloadIDst = reinterpret_cast<void *>(0x802d84a0);
        payloadISrc = &payloadPI;
        payloadISrcSize = payloadPI_size;
        payloadDSrc = &payloadPD;
        payloadDSrcSize = payloadPD_size;
        break;
    case 'E':
        payloadIDst = reinterpret_cast<void *>(0x802d8520);
        payloadISrc = &payloadEI;
        payloadISrcSize = payloadEI_size;
        payloadDSrc = &payloadED;
        payloadDSrcSize = payloadED_size;
        break;
    case 'J':
        payloadIDst = reinterpret_cast<void *>(0x802d8540);
        payloadISrc = &payloadJI;
        payloadISrcSize = payloadJI_size;
        payloadDSrc = &payloadJD;
        payloadDSrcSize = payloadJD_size;
        break;
    default:
        ERROR("Region detection failed!");
        return nullptr;
    }

    INFO("Decompressing payload instructions...");
    size_t payloadIDstSize = tinfl_decompress_mem_to_mem(payloadIDst,
            0x80360000 - reinterpret_cast<uintptr_t>(payloadIDst), payloadISrc, payloadISrcSize, 0);
    DCache::Flush(payloadIDst, payloadIDstSize);
    ICache::Invalidate(payloadIDst, payloadIDstSize);
    INFO("Decompressed payload instructions.");

    INFO("Decompressing payload data...");
    uintptr_t payloadDDstAddress = 0x80800000;
    void *payloadDDst = reinterpret_cast<void *>(payloadDDstAddress);
    size_t payloadDDstSize =
            tinfl_decompress_mem_to_mem(payloadDDst, 128 * 1024, payloadDSrc, payloadDSrcSize, 0);
    INFO("Decompressed payload data.");

    INFO("Copying common archive...");
    context->commonArchive = MEM1Arena::Instance()->alloc(commonArchive_size, -0x20);
    context->commonArchiveSize = commonArchive_size;
    memcpy(context->commonArchive, &commonArchive, commonArchive_size);
    INFO("Copied common archive.");

    INFO("Copying localized archive...");
    const u8 *localizedArchive;
    size_t localizedArchiveSize;
    GetLocalizedArchive(localizedArchive, localizedArchiveSize);
    context->localizedArchive = MEM1Arena::Instance()->alloc(localizedArchiveSize, -0x20);
    context->localizedArchiveSize = localizedArchiveSize;
    memcpy(context->localizedArchive, localizedArchive, localizedArchiveSize);
    INFO("Copied localized archive.");

    if (Platform::IsDolphin()) {
        // Enable OSReport over EXI
        consoleType |= 0x10000000;
    }
    arenaLo = payloadDDstAddress + payloadDDstSize;
    arenaHi = reinterpret_cast<uintptr_t>(MEM1Arena::Instance()->alloc(0x0, -0x4));

    INFO("Copying low memory...");
    uintptr_t lowMemoryDstAddress = 0x80400000;
    void *lowMemoryDst = reinterpret_cast<void *>(lowMemoryDstAddress);
    void *lowMemorySrc = reinterpret_cast<void *>(0x80000000);
    size_t lowMemorySize = 0x56c0;
    memcpy(lowMemoryDst, lowMemorySrc, lowMemorySize);
    INFO("Copied low memory.");

    INFO("Starting payload...");
    PayloadEntryFunc payloadEntry = reinterpret_cast<PayloadEntryFunc>(payloadIDst);
    return payloadEntry;
}

void Channel::RunApploader(Context *context) {
    while (true) {
        if (RunApploaderFromVirtualDI()) {
            context->hasVirtualDI = true;
            return;
        }
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

bool Channel::RunApploaderFromVirtualDI() {
    bool enableEXISD = true;
    bool enableUSB = !Platform::IsGameCube() && !Platform::IsDolphin();
    bool enableWiiSD = !Platform::IsGameCube();
    return RunApploaderFromVirtualDI(enableEXISD, enableUSB, enableWiiSD);
}

bool Channel::RunApploaderFromVirtualDI(bool enableEXISD, bool enableUSB, bool enableWiiSD) {
    if (enableEXISD) {
        EXISDStorage exiSDStorage0(0);
        EXISDStorage exiSDStorage1(1);
        EXISDStorage exiSDStorage2(2);
        EXISDStorage exiSDStorage3(3);

        return RunApploaderFromVirtualDI(false, enableUSB, enableWiiSD);
    }

    if (enableUSB) {
        USB::Handle usbHandle;

        return RunApploaderFromVirtualDI(enableEXISD, false, enableWiiSD);
    }

    if (enableWiiSD) {
        WiiSDStorage wiiSDStorage;

        return RunApploaderFromVirtualDI(enableEXISD, enableUSB, false);
    }

    if (!VirtualDI::Mount()) {
        return false;
    }

    if (!Apploader::Run(VirtualDI::Read)) {
        return false;
    }

    return true;
}

void Channel::GetLocalizedArchive(const u8 *&archive, size_t &archiveSize) {
    char region = DiscID::Get().gameID[3];
    if (region == 'J') {
        archive = japaneseArchive;
        archiveSize = japaneseArchive_size;
        return;
    }

    archive = englishArchive;
    archiveSize = englishArchive_size;

    if (region != 'P') {
        return;
    }

    SRAM sram;
    u8 language;
    if (!sram.getLanguage(language)) {
        return;
    }
    switch (language) {
    case SRAM::Language::English:
    case SRAM::Language::German:
    case SRAM::Language::French:
    case SRAM::Language::Spanish:
    case SRAM::Language::Italian:
        break;
    default:
        return;
    }

    const u8 *archives[] = {
            englishArchive,
            germanArchive,
            frenchArchive,
            spanishArchive,
            italianArchive,
    };
    size_t archiveSizes[] = {
            englishArchive_size,
            germanArchive_size,
            frenchArchive_size,
            spanishArchive_size,
            italianArchive_size,
    };
    archive = archives[language];
    archiveSize = archiveSizes[language];
}
