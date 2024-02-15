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
    INFO("Started payload.\n");

    INFO("Initializing IOS...\n");
    IOS::Resource::Init();
    INFO("Initialized IOS.\n");

    INFO("Initializing clock...\n");
    Clock::Init();
    INFO("Initialized clock.\n");

    INFO("Initializing WUP-028...\n");
    WUP028::Init();
    INFO("Initialized WUP-028.\n");

    INFO("Initializing assets directory creator...\n");
    AssetsDirCreator::Init();
    INFO("Initialized assets directory creator.\n");

    INFO("Initializing course manager...\n");
    CourseManager::Init();
    INFO("Initialized course manager.\n");

    INFO("Initializing virtual memory cards...\n");
    VirtualCard::Init();
    INFO("Initialized virtual memory cards.\n");

    INFO("Initializing storage...\n");
    Storage::Init();
    INFO("Initialized storage.\n");

    INFO("Initializing DVD storage...\n");
    DVDStorage::Init();
    INFO("Initialized DVD storage.\n");

    INFO("Initializing common archive storage...\n");
    ArchiveStorage::Init("carc:", context->commonArchive, context->commonArchiveSize);
    INFO("Initialized common archive storage.\n");

    INFO("Initializing localized archive storage...\n");
    ArchiveStorage::Init("larc:", context->localizedArchive, context->localizedArchiveSize);
    INFO("Initialized localized archive storage.\n");

    INFO("Initializing USB storage...\n");
    USBStorage::Init();
    INFO("Initialized USB storage.\n");

    INFO("Initializing USB...\n");
    USB::Init();
    INFO("Initialized USB.\n");

    INFO("Initializing SD storage...\n");
    SDStorage::Init();
    INFO("Initialized SD storage.\n");

    INFO("Initializing log file...\n");
    LogFile::Init();
    INFO("Initialized log file.\n");

    Console::Instance()->setIsDirect(false);
}
