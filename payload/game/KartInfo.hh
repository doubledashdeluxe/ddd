#pragma once

#include "game/CharDB.hh"
#include "game/KartDB.hh"
#include "game/KartGamePad.hh"

class KartInfo {
public:
    const KartDB *getKartDB() const;
    const CharDB *getCharDB(u32 characterIndex) const;

    static u32 GetKartWeight(u32 kartID);
    static u32 GetDriverWeight(u32 characterID);
    static u32 GetDefaultKartID(u32 characterID);

private:
    struct KartCharacter {
        KartGamePad *pad;
        const CharDB *charDB;
    };
    size_assert(KartCharacter, 0x8);

    const KartDB *m_kartDB;
    KartCharacter m_characters[2];
    u8 _04[0x18 - 0x14];
};
size_assert(KartInfo, 0x18);
