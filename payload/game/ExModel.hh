#pragma once

#include <dolphin/MTX.h>

class ExModel {
public:
    Mtx34 &baseTRMtx();

private:
    u8 _00[0x2c - 0x00];
    Mtx34 m_baseTRMtx;
    u8 _5c[0x8c - 0x5c];
};
size_assert(ExModel, 0x8c);
