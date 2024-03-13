#pragma once

#include <common/Types.hh>

class KartBody {
public:
    KartBody();

    u32 getGameStatus() const;
    u8 getIndex() const;

private:
    u8 _000[0x578 - 0x000];
    u32 m_gameStatus;
    u8 _57c[0x5b3 - 0x57c];
    u8 m_index;
    u8 _5b4[0x5c8 - 0x5b4];
};
size_assert(KartBody, 0x5c8);
