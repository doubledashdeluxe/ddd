#pragma once

#include <common/Array.hh>

class Apploader {
public:
    typedef bool (*ReadFunc)(void *dst, u32 size, u32 offset);

    typedef void (*GameEntryFunc)();

    static GameEntryFunc Run(ReadFunc read);

private:
    Apploader();

    typedef void (*ApploaderReportFunc)(const char *format, ...);

    typedef void (*ApploaderInitFunc)(ApploaderReportFunc);
    typedef s32 (*ApploaderMainFunc)(void **dst, u32 *size, u32 *shiftedOffset);
    typedef GameEntryFunc (*ApploaderCloseFunc)();

    typedef void (*ApploaderEntryFunc)(ApploaderInitFunc *init, ApploaderMainFunc *main,
            ApploaderCloseFunc *close);

    struct ApploaderHeader {
        char revision[0x10];
        ApploaderEntryFunc entry;
        u32 size;
        u32 trailer;
        u8 _1c[0x20 - 0x1c];
    };
    size_assert(ApploaderHeader, 0x20);

    static bool Read(ReadFunc read, void *dst, u32 size, u32 offset, const Array<u8, 32> *&hashes);
    static void Report(const char *format, ...);

    static const Array<u8, 32> HashesP[18];
    static const Array<u8, 32> HashesE[18];
    static const Array<u8, 32> HashesJ[18];
};
