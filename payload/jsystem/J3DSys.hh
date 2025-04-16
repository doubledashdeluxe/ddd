#pragma once

#include <portable/Types.hh>

class J3DSys {
public:
    void drawInit();

    static J3DSys &Instance();

private:
    u8 _000[0x058 - 0x000];

public:
    void *_058;

private:
    u8 _05c[0x10c - 0x05c];

public:
    u32 _10c;
    u32 _110;
    u32 _114;

    static J3DSys s_instance;
};
