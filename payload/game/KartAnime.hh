#pragma once

#include "portable/Types.hh"

class KartAnime {
public:
    bool isChangeStart() const;
    bool isGetItemAnime(u32 kartIndex) const;
    bool isBack() const;
    bool isBackStart() const;

private:
    u8 _00[0x14 - 0x00];
    u32 : 31;
    bool m_isChangeStart : 1;
    u8 _18[0x48 - 0x18];
};
size_assert(KartAnime, 0x48);
