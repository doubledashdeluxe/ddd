#include "Payload.hh"

#include "payload/Lock.hh"
#include "payload/LogFile.hh"
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

void Payload::Run() {
    VI::Init();

    Console::Init(VI::Instance());
    INFO("Double Dash Deluxe Payload\n");
    INFO("\n");

    INFO("Initializing IOS...");
    IOS::Resource::Init();
    INFO(" done.\n");

    INFO("Initializing clock...");
    Clock::Init();
    INFO(" done.\n");

    INFO("Initializing WUP-028...");
    WUP028::Init();
    INFO(" done.\n");

    INFO("Initializing storage...");
    Storage::Init();
    INFO(" done.\n");

    INFO("Initializing USB storage...");
    USBStorage::Init();
    INFO(" done.\n");

    INFO("Initializing USB...");
    USB::Init();
    INFO(" done.\n");

    INFO("Initializing SD storage...");
    SDStorage::Init();
    INFO(" done.\n");

    INFO("Initializing log file...");
    LogFile::Init();
    INFO(" done.\n");

    Console::Instance()->m_isActive = false;
}
