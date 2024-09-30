#pragma once

#include <common/Context.hh>

class Channel {
public:
    typedef void (*PayloadEntryFunc)(Context *context);

    static PayloadEntryFunc Run(Context *context);

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

    Channel();

    static void RunApploader(Context *context);
    static bool RunApploaderFromVirtualDI();
    static bool RunApploaderFromVirtualDI(bool enableUSB, bool enableSD);
    static u32 GetLanguage();
};
