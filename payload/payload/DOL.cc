#include "DOL.hh"

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

void *DOL::Start() {
    return dol_text_start;
}

void *DOL::End() {
    return dol_bss_end;
}

size_t DOL::Size() {
    return dol_bss_end - dol_text_start;
}

void *DOL::InitSectionStart() {
    return dol_init_start;
}

void *DOL::InitSectionEnd() {
    return dol_init_end;
}

size_t DOL::InitSectionSize() {
    return dol_init_end - dol_init_start;
}

void *DOL::ExtabSectionStart() {
    return dol_extab_start;
}

void *DOL::ExtabSectionEnd() {
    return dol_extab_end;
}

size_t DOL::ExtabSectionSize() {
    return dol_extab_end - dol_extab_start;
}

void *DOL::ExtabindexSectionStart() {
    return dol_extabindex_start;
}

void *DOL::ExtabindexSectionEnd() {
    return dol_extabindex_end;
}

size_t DOL::ExtabindexSectionSize() {
    return dol_extabindex_end - dol_extabindex_start;
}

void *DOL::TextSectionStart() {
    return dol_text_start;
}

void *DOL::TextSectionEnd() {
    return dol_text_end;
}

size_t DOL::TextSectionSize() {
    return dol_text_end - dol_text_start;
}

void *DOL::CtorsSectionStart() {
    return dol_ctors_start;
}

void *DOL::CtorsSectionEnd() {
    return dol_ctors_end;
}

size_t DOL::CtorsSectionSize() {
    return dol_ctors_end - dol_ctors_start;
}

void *DOL::DtorsSectionStart() {
    return dol_dtors_start;
}

void *DOL::DtorsSectionEnd() {
    return dol_dtors_end;
}

size_t DOL::DtorsSectionSize() {
    return dol_dtors_end - dol_dtors_start;
}

void *DOL::BinarySectionStart() {
    return dol_BINARY_start;
}

void *DOL::BinarySectionEnd() {
    return dol_BINARY_end;
}

size_t DOL::BinarySectionSize() {
    return dol_BINARY_end - dol_BINARY_start;
}

void *DOL::RodataSectionStart() {
    return dol_rodata_start;
}

void *DOL::RodataSectionEnd() {
    return dol_rodata_end;
}

size_t DOL::RodataSectionSize() {
    return dol_rodata_end - dol_rodata_start;
}

void *DOL::DataSectionStart() {
    return dol_data_start;
}

void *DOL::DataSectionEnd() {
    return dol_data_end;
}

size_t DOL::DataSectionSize() {
    return dol_data_end - dol_data_start;
}

void *DOL::BssSectionStart() {
    return dol_bss_start;
}

void *DOL::BssSectionEnd() {
    return dol_bss_end;
}

size_t DOL::BssSectionSize() {
    return dol_bss_end - dol_bss_start;
}

void *DOL::SdataSectionStart() {
    return dol_sdata_start;
}

void *DOL::SdataSectionEnd() {
    return dol_sdata_end;
}

size_t DOL::SdataSectionSize() {
    return dol_sdata_end - dol_sdata_start;
}

void *DOL::SbssSectionStart() {
    return dol_sbss_start;
}

void *DOL::SbssSectionEnd() {
    return dol_sbss_end;
}

size_t DOL::SbssSectionSize() {
    return dol_sbss_end - dol_sbss_start;
}

void *DOL::Sdata2SectionStart() {
    return dol_sdata2_start;
}

void *DOL::Sdata2SectionEnd() {
    return dol_sdata2_end;
}

size_t DOL::Sdata2SectionSize() {
    return dol_sdata2_end - dol_sdata2_start;
}

void *DOL::Sbss2SectionStart() {
    return dol_sbss2_start;
}

void *DOL::Sbss2SectionEnd() {
    return dol_sbss2_end;
}

size_t DOL::Sbss2SectionSize() {
    return dol_sbss2_end - dol_sbss2_start;
}
