#include "Payload.hh"

#include "payload/ArchiveStorage.hh"
#include "payload/AssetsDirCreator.hh"
#include "payload/CourseManager.hh"
#include "payload/DVDStorage.hh"
#include "payload/Lock.hh"
#include "payload/LogFile.hh"
#include "payload/VirtualCard.hh"
#include "payload/WUP028.hh"

#include <common/Clock.hh>
#include <common/Console.hh>
#include <common/Log.hh>
#include <common/Platform.hh>
#include <common/USB.hh>
#include <common/ios/Resource.hh>
#include <common/storage/SDStorage.hh>
#include <common/storage/Storage.hh>
#include <common/storage/USBStorage.hh>

extern "C" u8 payload_text_start[];
extern "C" u8 payload_text_end[];
extern "C" u8 payload_ctors_start[];
extern "C" u8 payload_ctors_end[];
extern "C" u8 payload_dtors_start[];
extern "C" u8 payload_dtors_end[];
extern "C" u8 payload_rodata_start[];
extern "C" u8 payload_rodata_end[];
extern "C" u8 payload_data_start[];
extern "C" u8 payload_data_end[];
extern "C" u8 payload_bss_start[];
extern "C" u8 payload_bss_end[];

void *Payload::Start() {
    return payload_text_start;
}

void *Payload::End() {
    return payload_bss_end;
}

size_t Payload::Size() {
    return payload_bss_end - payload_text_start;
}

void *Payload::TextSectionStart() {
    return payload_text_start;
}

void *Payload::TextSectionEnd() {
    return payload_text_end;
}

size_t Payload::TextSectionSize() {
    return payload_text_end - payload_text_start;
}

void *Payload::CtorsSectionStart() {
    return payload_ctors_start;
}

void *Payload::CtorsSectionEnd() {
    return payload_ctors_end;
}

size_t Payload::CtorsSectionSize() {
    return payload_ctors_end - payload_ctors_start;
}

void *Payload::DtorsSectionStart() {
    return payload_dtors_start;
}

void *Payload::DtorsSectionEnd() {
    return payload_dtors_end;
}

size_t Payload::DtorsSectionSize() {
    return payload_dtors_end - payload_dtors_start;
}

void *Payload::RodataSectionStart() {
    return payload_rodata_start;
}

void *Payload::RodataSectionEnd() {
    return payload_rodata_end;
}

size_t Payload::RodataSectionSize() {
    return payload_rodata_end - payload_rodata_start;
}

void *Payload::DataSectionStart() {
    return payload_data_start;
}

void *Payload::DataSectionEnd() {
    return payload_data_end;
}

size_t Payload::DataSectionSize() {
    return payload_data_end - payload_data_start;
}

void *Payload::BssSectionStart() {
    return payload_bss_start;
}

void *Payload::BssSectionEnd() {
    return payload_bss_end;
}

size_t Payload::BssSectionSize() {
    return payload_bss_end - payload_bss_start;
}

void Payload::Run(Context *context) {
    INFO("Started payload.");

    INFO("Initializing IOS...");
    IOS::Resource::Init();
    INFO("Initialized IOS.");

    INFO("Initializing clock...");
    Clock::Init();
    INFO("Initialized clock.");

    INFO("Initializing WUP-028...");
    WUP028::Init();
    INFO("Initialized WUP-028.");

    INFO("Initializing assets directory creator...");
    AssetsDirCreator::Init();
    INFO("Initialized assets directory creator.");

    INFO("Initializing course manager...");
    CourseManager::Init();
    INFO("Initialized course manager.");

    INFO("Initializing virtual memory cards...");
    VirtualCard::Init();
    INFO("Initialized virtual memory cards.");

    INFO("Initializing storage...");
    Storage::Init();
    INFO("Initialized storage.");

    INFO("Initializing DVD storage...");
    DVDStorage::Init();
    INFO("Initialized DVD storage.");

    INFO("Initializing common archive storage...");
    ArchiveStorage::Init("carc:", context->commonArchive, context->commonArchiveSize);
    INFO("Initialized common archive storage.");

    INFO("Initializing localized archive storage...");
    ArchiveStorage::Init("larc:", context->localizedArchive, context->localizedArchiveSize);
    INFO("Initialized localized archive storage.");

    INFO("Initializing USB storage...");
    USBStorage::Init();
    INFO("Initialized USB storage.");

    INFO("Initializing USB...");
    USB::Init();
    INFO("Initialized USB.");

    INFO("Initializing SD storage...");
    SDStorage::Init();
    INFO("Initialized SD storage.");

    INFO("Initializing log file...");
    LogFile::Init();
    INFO("Initialized log file.");

    Console::Instance()->setIsDirect(false);
}
