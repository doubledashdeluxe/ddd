#pragma once

#include "game/KartBody.hh"

class KartSound {
public:
    void REPLACED(doHorn)();
    REPLACE void doHorn();

private:
    u8 _0[0x4 - 0x0];
    KartBody *m_body;
    u8 _8[0xc - 0x8];
};
size_assert(KartSound, 0xc);
