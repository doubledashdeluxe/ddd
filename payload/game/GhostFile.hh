#pragma once

#include "game/KartPadRecord.hh"
#include "game/SaveFile.hh"

#include <payload/Replace.hh>

class GhostFile : public SaveFile {
public:
    REPLACE bool updateKartPadRecord(const KartPadRecord &padRecord);

private:
    u8 _0004[0x1460 - 0x0004];
    KartPadRecord m_padRecord;
    u8 _a018[0xa048 - 0xa018];
    s32 m_ghostType;
    u8 _a04c[0xa060 - 0xa04c];
};
size_assert(GhostFile, 0xa060);
