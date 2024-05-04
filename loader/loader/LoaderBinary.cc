#include "LoaderBinary.hh"

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

void *LoaderBinary::Start() {
    return loader_text_start;
}

void *LoaderBinary::End() {
    return loader_bss_end;
}

size_t LoaderBinary::Size() {
    return loader_bss_end - loader_text_start;
}

void *LoaderBinary::TextSectionStart() {
    return loader_text_start;
}

void *LoaderBinary::TextSectionEnd() {
    return loader_text_end;
}

size_t LoaderBinary::TextSectionSize() {
    return loader_text_end - loader_text_start;
}

void *LoaderBinary::CtorsSectionStart() {
    return loader_ctors_start;
}

void *LoaderBinary::CtorsSectionEnd() {
    return loader_ctors_end;
}

size_t LoaderBinary::CtorsSectionSize() {
    return loader_ctors_end - loader_ctors_start;
}

void *LoaderBinary::DtorsSectionStart() {
    return loader_dtors_start;
}

void *LoaderBinary::DtorsSectionEnd() {
    return loader_dtors_end;
}

size_t LoaderBinary::DtorsSectionSize() {
    return loader_dtors_end - loader_dtors_start;
}

void *LoaderBinary::RodataSectionStart() {
    return loader_rodata_start;
}

void *LoaderBinary::RodataSectionEnd() {
    return loader_rodata_end;
}

size_t LoaderBinary::RodataSectionSize() {
    return loader_rodata_end - loader_rodata_start;
}

void *LoaderBinary::DataSectionStart() {
    return loader_data_start;
}

void *LoaderBinary::DataSectionEnd() {
    return loader_data_end;
}

size_t LoaderBinary::DataSectionSize() {
    return loader_data_end - loader_data_start;
}

void *LoaderBinary::BssSectionStart() {
    return loader_bss_start;
}

void *LoaderBinary::BssSectionEnd() {
    return loader_bss_end;
}

size_t LoaderBinary::BssSectionSize() {
    return loader_bss_end - loader_bss_start;
}
