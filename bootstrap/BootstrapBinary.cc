#include "BootstrapBinary.hh"

extern "C" u8 bootstrap_text_start[];
extern "C" u8 bootstrap_text_end[];
extern "C" u8 bootstrap_ctors_start[];
extern "C" u8 bootstrap_ctors_end[];
extern "C" u8 bootstrap_dtors_start[];
extern "C" u8 bootstrap_dtors_end[];
extern "C" u8 bootstrap_rodata_start[];
extern "C" u8 bootstrap_rodata_end[];
extern "C" u8 bootstrap_data_start[];
extern "C" u8 bootstrap_data_end[];
extern "C" u8 bootstrap_bss_start[];
extern "C" u8 bootstrap_bss_end[];

void *BootstrapBinary::Start() {
    return bootstrap_text_start;
}

void *BootstrapBinary::End() {
    return bootstrap_bss_end;
}

size_t BootstrapBinary::Size() {
    return bootstrap_bss_end - bootstrap_text_start;
}

void *BootstrapBinary::TextSectionStart() {
    return bootstrap_text_start;
}

void *BootstrapBinary::TextSectionEnd() {
    return bootstrap_text_end;
}

size_t BootstrapBinary::TextSectionSize() {
    return bootstrap_text_end - bootstrap_text_start;
}

void *BootstrapBinary::CtorsSectionStart() {
    return bootstrap_ctors_start;
}

void *BootstrapBinary::CtorsSectionEnd() {
    return bootstrap_ctors_end;
}

size_t BootstrapBinary::CtorsSectionSize() {
    return bootstrap_ctors_end - bootstrap_ctors_start;
}

void *BootstrapBinary::DtorsSectionStart() {
    return bootstrap_dtors_start;
}

void *BootstrapBinary::DtorsSectionEnd() {
    return bootstrap_dtors_end;
}

size_t BootstrapBinary::DtorsSectionSize() {
    return bootstrap_dtors_end - bootstrap_dtors_start;
}

void *BootstrapBinary::RodataSectionStart() {
    return bootstrap_rodata_start;
}

void *BootstrapBinary::RodataSectionEnd() {
    return bootstrap_rodata_end;
}

size_t BootstrapBinary::RodataSectionSize() {
    return bootstrap_rodata_end - bootstrap_rodata_start;
}

void *BootstrapBinary::DataSectionStart() {
    return bootstrap_data_start;
}

void *BootstrapBinary::DataSectionEnd() {
    return bootstrap_data_end;
}

size_t BootstrapBinary::DataSectionSize() {
    return bootstrap_data_end - bootstrap_data_start;
}

void *BootstrapBinary::BssSectionStart() {
    return bootstrap_bss_start;
}

void *BootstrapBinary::BssSectionEnd() {
    return bootstrap_bss_end;
}

size_t BootstrapBinary::BssSectionSize() {
    return bootstrap_bss_end - bootstrap_bss_start;
}
