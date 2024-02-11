#pragma once

#include "game/KartPadRecord.hh"

#include <payload/Replace.hh>

class GhostFile {
public:
    REPLACE bool updateKartPadRecord(const KartPadRecord &padRecord);

private:
    u8 _0000[0x1460 - 0x0000];
    KartPadRecord m_padRecord;
    u8 _a018[0xa048 - 0xa018];
    s32 m_ghostType;
    u8 _a04c[0xa060 - 0xa04c];
};
size_assert(GhostFile, 0xa060);
