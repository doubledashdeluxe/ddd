#pragma once

#include <common/Types.hh>

class Apploader {
public:
    typedef void (*GameEntryFunc)();

    static GameEntryFunc LoadAndRun();

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
    static_assert(sizeof(ApploaderHeader) == 0x20);

    static bool IsDiscIDValid();
    static void Report(const char *format, ...);
};
