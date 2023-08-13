#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

class LogoApp {
public:
    REPLACE static void Call();

private:
    LogoApp();

    u8 _00[0x3c - 0x00];
};
static_assert(sizeof(LogoApp) == 0x3c);
