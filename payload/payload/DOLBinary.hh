#pragma once

#include <portable/Types.hh>

class DOLBinary {
public:
    static void *Start();
    static void *End();
    static size_t Size();
    static void *InitSectionStart();
    static void *InitSectionEnd();
    static size_t InitSectionSize();
    static void *ExtabSectionStart();
    static void *ExtabSectionEnd();
    static size_t ExtabSectionSize();
    static void *ExtabindexSectionStart();
    static void *ExtabindexSectionEnd();
    static size_t ExtabindexSectionSize();
    static void *TextSectionStart();
    static void *TextSectionEnd();
    static size_t TextSectionSize();
    static void *CtorsSectionStart();
    static void *CtorsSectionEnd();
    static size_t CtorsSectionSize();
    static void *DtorsSectionStart();
    static void *DtorsSectionEnd();
    static size_t DtorsSectionSize();
    static void *BinarySectionStart();
    static void *BinarySectionEnd();
    static size_t BinarySectionSize();
    static void *RodataSectionStart();
    static void *RodataSectionEnd();
    static size_t RodataSectionSize();
    static void *DataSectionStart();
    static void *DataSectionEnd();
    static size_t DataSectionSize();
    static void *BssSectionStart();
    static void *BssSectionEnd();
    static size_t BssSectionSize();
    static void *SdataSectionStart();
    static void *SdataSectionEnd();
    static size_t SdataSectionSize();
    static void *SbssSectionStart();
    static void *SbssSectionEnd();
    static size_t SbssSectionSize();
    static void *Sdata2SectionStart();
    static void *Sdata2SectionEnd();
    static size_t Sdata2SectionSize();
    static void *Sbss2SectionStart();
    static void *Sbss2SectionEnd();
    static size_t Sbss2SectionSize();

private:
    DOLBinary();
};
