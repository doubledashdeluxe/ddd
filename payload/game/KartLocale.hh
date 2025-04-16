#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class KartLocale {
public:
    class Language {
    public:
        enum {
            English = 0,
            French = 1,
            German = 2,
            Italian = 3,
            Japanese = 4,
            Spanish = 5,
            Count = 6,
        };

    private:
        Language();
    };

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
    static u32 GetLanguage();
    static const char *GetLanguageName();
    static const char *GetLanguageName(u32 language);
    static u32 GetVideoFrameMode();

private:
    KartLocale();

    static u32 s_language;
    static u32 s_videoMode;
    static u32 s_videoFrameMode;

    static const char *const LanguageNames[6];
};
size_assert(KartLocale, 0x1);
