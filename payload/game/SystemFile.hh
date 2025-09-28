#pragma once

#include "game/ExtendedSystemRecord.hh"
#include "game/SaveFile.hh"

#include <payload/Replace.hh>

class SystemFile : public SaveFile {
public:
    REPLACE void setPart(s32 filePart) override;
    void REPLACED(fetch)();
    REPLACE void fetch() override;
    void REPLACED(store)();
    REPLACE void store() override;

private:
    u8 _0004[0x1c20 - 0x0004];
    ExtendedSystemRecord m_extendedSystemRecord; // Added (was padding)
    u8 _1c2c[0x6028 - 0x1c5c];
    s32 m_filePart;
    u8 _602c[0x6040 - 0x602c];
};
size_assert(SystemFile, 0x6040);
