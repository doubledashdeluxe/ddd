#include "DOLBinary.hh"

extern "C" u8 dol_init_start[];
extern "C" u8 dol_init_end[];
extern "C" u8 dol_extab_start[];
extern "C" u8 dol_extab_end[];
extern "C" u8 dol_extabindex_start[];
extern "C" u8 dol_extabindex_end[];
extern "C" u8 dol_text_start[];
extern "C" u8 dol_text_end[];
extern "C" u8 dol_ctors_start[];
extern "C" u8 dol_ctors_end[];
extern "C" u8 dol_dtors_start[];
extern "C" u8 dol_dtors_end[];
extern "C" u8 dol_BINARY_start[];
extern "C" u8 dol_BINARY_end[];
extern "C" u8 dol_rodata_start[];
extern "C" u8 dol_rodata_end[];
extern "C" u8 dol_data_start[];
extern "C" u8 dol_data_end[];
extern "C" u8 dol_bss_start[];
extern "C" u8 dol_bss_end[];
extern "C" u8 dol_sdata_start[];
extern "C" u8 dol_sdata_end[];
extern "C" u8 dol_sbss_start[];
extern "C" u8 dol_sbss_end[];
extern "C" u8 dol_sdata2_start[];
extern "C" u8 dol_sdata2_end[];
extern "C" u8 dol_sbss2_start[];
extern "C" u8 dol_sbss2_end[];

void *DOLBinary::Start() {
    return dol_text_start;
}

void *DOLBinary::End() {
    return dol_sbss2_end;
}

size_t DOLBinary::Size() {
    return dol_sbss2_end - dol_text_start;
}

void *DOLBinary::InitSectionStart() {
    return dol_init_start;
}

void *DOLBinary::InitSectionEnd() {
    return dol_init_end;
}

size_t DOLBinary::InitSectionSize() {
    return dol_init_end - dol_init_start;
}

void *DOLBinary::ExtabSectionStart() {
    return dol_extab_start;
}

void *DOLBinary::ExtabSectionEnd() {
    return dol_extab_end;
}

size_t DOLBinary::ExtabSectionSize() {
    return dol_extab_end - dol_extab_start;
}

void *DOLBinary::ExtabindexSectionStart() {
    return dol_extabindex_start;
}

void *DOLBinary::ExtabindexSectionEnd() {
    return dol_extabindex_end;
}

size_t DOLBinary::ExtabindexSectionSize() {
    return dol_extabindex_end - dol_extabindex_start;
}

void *DOLBinary::TextSectionStart() {
    return dol_text_start;
}

void *DOLBinary::TextSectionEnd() {
    return dol_text_end;
}

size_t DOLBinary::TextSectionSize() {
    return dol_text_end - dol_text_start;
}

void *DOLBinary::CtorsSectionStart() {
    return dol_ctors_start;
}

void *DOLBinary::CtorsSectionEnd() {
    return dol_ctors_end;
}

size_t DOLBinary::CtorsSectionSize() {
    return dol_ctors_end - dol_ctors_start;
}

void *DOLBinary::DtorsSectionStart() {
    return dol_dtors_start;
}

void *DOLBinary::DtorsSectionEnd() {
    return dol_dtors_end;
}

size_t DOLBinary::DtorsSectionSize() {
    return dol_dtors_end - dol_dtors_start;
}

void *DOLBinary::BinarySectionStart() {
    return dol_BINARY_start;
}

void *DOLBinary::BinarySectionEnd() {
    return dol_BINARY_end;
}

size_t DOLBinary::BinarySectionSize() {
    return dol_BINARY_end - dol_BINARY_start;
}

void *DOLBinary::RodataSectionStart() {
    return dol_rodata_start;
}

void *DOLBinary::RodataSectionEnd() {
    return dol_rodata_end;
}

size_t DOLBinary::RodataSectionSize() {
    return dol_rodata_end - dol_rodata_start;
}

void *DOLBinary::DataSectionStart() {
    return dol_data_start;
}

void *DOLBinary::DataSectionEnd() {
    return dol_data_end;
}

size_t DOLBinary::DataSectionSize() {
    return dol_data_end - dol_data_start;
}

void *DOLBinary::BssSectionStart() {
    return dol_bss_start;
}

void *DOLBinary::BssSectionEnd() {
    return dol_bss_end;
}

size_t DOLBinary::BssSectionSize() {
    return dol_bss_end - dol_bss_start;
}

void *DOLBinary::SdataSectionStart() {
    return dol_sdata_start;
}

void *DOLBinary::SdataSectionEnd() {
    return dol_sdata_end;
}

size_t DOLBinary::SdataSectionSize() {
    return dol_sdata_end - dol_sdata_start;
}

void *DOLBinary::SbssSectionStart() {
    return dol_sbss_start;
}

void *DOLBinary::SbssSectionEnd() {
    return dol_sbss_end;
}

size_t DOLBinary::SbssSectionSize() {
    return dol_sbss_end - dol_sbss_start;
}

void *DOLBinary::Sdata2SectionStart() {
    return dol_sdata2_start;
}

void *DOLBinary::Sdata2SectionEnd() {
    return dol_sdata2_end;
}

size_t DOLBinary::Sdata2SectionSize() {
    return dol_sdata2_end - dol_sdata2_start;
}

void *DOLBinary::Sbss2SectionStart() {
    return dol_sbss2_start;
}

void *DOLBinary::Sbss2SectionEnd() {
    return dol_sbss2_end;
}

size_t DOLBinary::Sbss2SectionSize() {
    return dol_sbss2_end - dol_sbss2_start;
}
