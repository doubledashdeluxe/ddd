#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class JUTProcBar {
public:
    REPLACE void draw();

private:
    u8 _000[0x134 - 0x000];
};
size_assert(JUTProcBar, 0x134);
