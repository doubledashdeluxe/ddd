#pragma once

#include "game/KartBody.hh"

class KartItem {
private:
    void REPLACED(doOtherAnime)();
    REPLACE void doOtherAnime();
    void REPLACED(doTandemItemAnime)();
    REPLACE void doTandemItemAnime();
    void REPLACED(doTandemItemRelease)();
    REPLACE void doTandemItemRelease();

    static bool IsTurtle(u32 itemID);
    static bool IsBanana(u32 itemID);

    KartBody *m_body;
    u8 _4[0xc - 0x4];
};
size_assert(KartItem, 0xc);
