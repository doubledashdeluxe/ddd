#pragma once

#include <common/Context.hh>

class Loader {
public:
    typedef void (*PayloadEntryFunc)(Context *context);

    static PayloadEntryFunc Run(Context *context);

    static void *Start();
    static void *End();
    static size_t Size();
    static void *TextSectionStart();
    static void *TextSectionEnd();
    static size_t TextSectionSize();
    static void *CtorsSectionStart();
    static void *CtorsSectionEnd();
    static size_t CtorsSectionSize();
    static void *DtorsSectionStart();
    static void *DtorsSectionEnd();
    static size_t DtorsSectionSize();
    static void *RodataSectionStart();
    static void *RodataSectionEnd();
    static size_t RodataSectionSize();
    static void *DataSectionStart();
    static void *DataSectionEnd();
    static size_t DataSectionSize();
    static void *BssSectionStart();
    static void *BssSectionEnd();
    static size_t BssSectionSize();

private:
    class Language {
    public:
        enum {
            Japanese = 0,
            English = 1,
            German = 2,
            French = 3,
            Spanish = 4,
            Italian = 5,
        };

    private:
        Language();
    };

    Loader();

    static bool IsDiscIDValid();
    static u32 GetLanguage();
};
