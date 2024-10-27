#pragma once

#include "game/KartDB.hh"

class KartInfo {
public:
    const KartDB *getKartDB() const;

private:
    const KartDB *m_kartDB;
    u8 _04[0x18 - 0x04];
};
size_assert(KartInfo, 0x18);
