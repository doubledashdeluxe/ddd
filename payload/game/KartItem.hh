#pragma once

#include "game/KartBody.hh"

class KartItem {
public:
    void REPLACED(setSucMashItem)();
    REPLACE void(setSucMashItem)();
    bool REPLACED(isMyItem)() const;
    REPLACE bool isMyItem() const;

private:
    void REPLACED(doTandemItemAnime)();
    REPLACE void doTandemItemAnime();
    void REPLACED(doTandemItemRelease)();
    REPLACE void doTandemItemRelease();

    static bool IsTurtle(u32 itemID);
    static bool IsBanana(u32 itemID);

    KartBody *m_body;
    bool m_taMushroomDone;
    u16 m_taMushroomFrame;
    u8 _8[0xc - 0x8];
};
size_assert(KartItem, 0xc);
