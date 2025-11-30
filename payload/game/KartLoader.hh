#pragma once

#include "game/ExModel.hh"

class KartLoader {
public:
    ExModel &bodyModel();

private:
    u8 _000[0x014 - 0x000];
    ExModel m_bodyModel;
    u8 _0a0[0xb80 - 0x0a0];
};
size_assert(KartLoader, 0xb80);
