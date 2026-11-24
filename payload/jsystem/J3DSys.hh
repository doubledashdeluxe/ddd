#pragma once

extern "C" {
#include <dolphin/MTX.h>
}
#include <portable/Types.hh>

class J3DSys {
public:
    void setTexCacheRegion(u32 cacheSize);
    void drawInit();
    void reinitGX();
    void setViewMtx(const Mtx34 viewMtx);

    static J3DSys &Instance();

private:
    Mtx34 m_viewMtx;
    u8 _030[0x058 - 0x030];

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
