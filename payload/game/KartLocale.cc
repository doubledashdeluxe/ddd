#include "KartLocale.hh"

extern "C" u32 videoMode;

extern "C" volatile u16 dcr;

void KartLocale::Localize() {
    REPLACED(Localize)();

    s_videoMode = videoMode == 0 ? VideoMode::NTSC : VideoMode::PAL;
    s_videoFrameMode = (dcr >> 8 & 3) == 0 ? VideoFrameMode::SixtyHz : VideoFrameMode::FiftyHz;
}

u32 KartLocale::GetLanguage() {
    return s_language;
}

const char *KartLocale::GetLanguageName() {
    return LanguageNames[s_language];
}

const char *KartLocale::GetLanguageName(u32 language) {
    return LanguageNames[language];
}

u32 KartLocale::GetVideoFrameMode() {
    return s_videoFrameMode;
}
