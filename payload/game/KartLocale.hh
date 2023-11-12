#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

class KartLocale {
public:
    class VideoMode {
    public:
        enum {
            NTSC = 0,
            PAL = 1,
        };

    private:
        VideoMode();
    };

    class VideoFrameMode {
    public:
        enum {
            SixtyHz = 0,
            FiftyHz = 1,
        };

    private:
        VideoFrameMode();
    };

    static void REPLACED(Localize)();
    REPLACE static void Localize();
    static const char *LanguageName();

private:
    KartLocale();

    static u32 s_language;
    static u32 s_videoMode;
    static u32 s_videoFrameMode;

    static const char *const LanguageNames[6];
};
size_assert(KartLocale, 0x1);
