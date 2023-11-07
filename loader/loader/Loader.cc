#include "Loader.hh"

#include "loader/Apploader.hh"
#include "loader/DI.hh"

#include <common/Arena.hh>
#include <common/Clock.hh>
#include <common/DCache.hh>
#include <common/ICache.hh>
#include <common/Log.hh>
#include <common/Platform.hh>

extern "C" {
#include <string.h>
}

extern "C" u8 loader_text_start[];
extern "C" u8 loader_text_end[];
extern "C" u8 loader_ctors_start[];
extern "C" u8 loader_ctors_end[];
extern "C" u8 loader_dtors_start[];
extern "C" u8 loader_dtors_end[];
extern "C" u8 loader_rodata_start[];
extern "C" u8 loader_rodata_end[];
extern "C" u8 loader_data_start[];
extern "C" u8 loader_data_end[];
extern "C" u8 loader_bss_start[];
extern "C" u8 loader_bss_end[];

extern "C" const u8 payloadP[];
extern "C" const size_t payloadP_size;
extern "C" const u8 payloadE[];
extern "C" const size_t payloadE_size;
extern "C" const u8 payloadJ[];
extern "C" const size_t payloadJ_size;

extern "C" const u8 commonArchive[];
extern "C" const size_t commonArchive_size;

extern "C" u32 discID[8];
extern "C" u32 consoleType;
extern "C" u32 arenaLo;
extern "C" u32 iosVersion;

extern "C" volatile u32 aicr;

extern "C" volatile u32 armirqmask;
extern "C" volatile u32 aipprot;

void *Loader::Start() {
    return loader_text_start;
}

void *Loader::End() {
    return loader_bss_end;
}

size_t Loader::Size() {
    return loader_bss_end - loader_text_start;
}

void *Loader::TextSectionStart() {
    return loader_text_start;
}

void *Loader::TextSectionEnd() {
    return loader_text_end;
}

size_t Loader::TextSectionSize() {
    return loader_text_end - loader_text_start;
}

void *Loader::CtorsSectionStart() {
    return loader_ctors_start;
}

void *Loader::CtorsSectionEnd() {
    return loader_ctors_end;
}

size_t Loader::CtorsSectionSize() {
    return loader_ctors_end - loader_ctors_start;
}

void *Loader::DtorsSectionStart() {
    return loader_dtors_start;
}

void *Loader::DtorsSectionEnd() {
    return loader_dtors_end;
}

size_t Loader::DtorsSectionSize() {
    return loader_dtors_end - loader_dtors_start;
}

void *Loader::RodataSectionStart() {
    return loader_rodata_start;
}

void *Loader::RodataSectionEnd() {
    return loader_rodata_end;
}

size_t Loader::RodataSectionSize() {
    return loader_rodata_end - loader_rodata_start;
}

void *Loader::DataSectionStart() {
    return loader_data_start;
}

void *Loader::DataSectionEnd() {
    return loader_data_end;
}

size_t Loader::DataSectionSize() {
    return loader_data_end - loader_data_start;
}

void *Loader::BssSectionStart() {
    return loader_bss_start;
}

void *Loader::BssSectionEnd() {
    return loader_bss_end;
}

size_t Loader::BssSectionSize() {
    return loader_bss_end - loader_bss_start;
}

Loader::PayloadEntryFunc Loader::Run(Context *context) {
    // Use compat MMIO ranges for DI/SI/EXI/AI
    aipprot &= ~0x1;

    // Prevent Starlet from receiving DI interrupts
    armirqmask &= ~(1 << 18);

    // Reset the DSP: libogc apps like the HBC cannot initialize it properly, but the SDK can.
    aicr = 0;

    VI::Init();

    Console::Init(VI::Instance());
    INFO("Double Dash Deluxe Loader\n");
    INFO("\n");

    if (iosVersion >> 16 != 58 && iosVersion >> 16 != 59) {
        ERROR("In order for Double Dash Deluxe to work, IOS58 (or IOS59) must be installed.\n");
        ERROR("Please perform a Wii System Update or use the IOS58 Installer to install IOS58.\n");
        return nullptr;
    }

    if (Platform::IsDolphin()) {
        Array<char, 64> dolphinVersion;
        if (!Platform::GetDolphinVersion(dolphinVersion) ||
                strncmp(dolphinVersion.values(), "5.0-20189", strlen("5.0-20189")) < 0) {
            WARN("Using DSP HLE will result in broken sound on this Dolphin version!\n");
            WARN("Please update to Dolphin 5.0-20189 or later.\n");
            WARN("\n");
            Clock::WaitMilliseconds(1000);
        }
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

    INFO("Copying common archive...");
    context->commonArchive = MEM2Arena::Instance()->alloc(commonArchive_size, 0x20);
    context->commonArchiveSize = commonArchive_size;
    memcpy(context->commonArchive, &commonArchive, commonArchive_size);
    INFO(" done.\n");

    if (Platform::IsDolphin()) {
        // Enable OSReport over EXI
        consoleType = 0x10000006;
    }
    arenaLo = reinterpret_cast<uintptr_t>(payloadDst) + payloadSize;

    INFO("Starting payload...");
    PayloadEntryFunc payloadEntry = reinterpret_cast<PayloadEntryFunc>(payloadDst);
    return payloadEntry;
}
