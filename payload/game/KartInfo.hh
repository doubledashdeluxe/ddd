#pragma once

#include "game/KartDB.hh"

class KartInfo {
public:
    const KartDB *getKartDB() const;

    static u32 GetKartWeight(u32 kartID);
    static u32 GetDriverWeight(u32 characterID);
    static u32 GetDefaultKartID(u32 characterID);

private:
    const KartDB *m_kartDB;
    u8 _04[0x18 - 0x04];
};
size_assert(KartInfo, 0x18);
