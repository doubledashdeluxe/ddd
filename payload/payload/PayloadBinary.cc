#include "PayloadBinary.hh"

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

void *PayloadBinary::Start() {
    return payload_text_start;
}

void *PayloadBinary::End() {
    return payload_bss_end;
}

size_t PayloadBinary::Size() {
    return payload_bss_end - payload_text_start;
}

void *PayloadBinary::TextSectionStart() {
    return payload_text_start;
}

void *PayloadBinary::TextSectionEnd() {
    return payload_text_end;
}

size_t PayloadBinary::TextSectionSize() {
    return payload_text_end - payload_text_start;
}

void *PayloadBinary::CtorsSectionStart() {
    return payload_ctors_start;
}

void *PayloadBinary::CtorsSectionEnd() {
    return payload_ctors_end;
}

size_t PayloadBinary::CtorsSectionSize() {
    return payload_ctors_end - payload_ctors_start;
}

void *PayloadBinary::DtorsSectionStart() {
    return payload_dtors_start;
}

void *PayloadBinary::DtorsSectionEnd() {
    return payload_dtors_end;
}

size_t PayloadBinary::DtorsSectionSize() {
    return payload_dtors_end - payload_dtors_start;
}

void *PayloadBinary::RodataSectionStart() {
    return payload_rodata_start;
}

void *PayloadBinary::RodataSectionEnd() {
    return payload_rodata_end;
}

size_t PayloadBinary::RodataSectionSize() {
    return payload_rodata_end - payload_rodata_start;
}

void *PayloadBinary::DataSectionStart() {
    return payload_data_start;
}

void *PayloadBinary::DataSectionEnd() {
    return payload_data_end;
}

size_t PayloadBinary::DataSectionSize() {
    return payload_data_end - payload_data_start;
}

void *PayloadBinary::BssSectionStart() {
    return payload_bss_start;
}

void *PayloadBinary::BssSectionEnd() {
    return payload_bss_end;
}

size_t PayloadBinary::BssSectionSize() {
    return payload_bss_end - payload_bss_start;
}
