#pragma once

#include <portable/Types.hh>

class KartAnime {
public:
    bool isChangeStart() const;
    bool isGetItemAnime(u32 kartIndex) const;
    bool isBack(u32 kartIndex) const;
    bool isBackStart(u32 kartIndex) const;
    void makeThrowAnime(bool isBack, bool isItemBack, bool isSuccessionItem);
    void makeCrouchAnime();

private:
    u8 _00[0x14 - 0x00];
    u32 : 22;
    bool m_isCrouch : 1;
    u32 : 8;
    bool m_isChangeStart : 1;
    u8 _18[0x48 - 0x18];
};
size_assert(KartAnime, 0x48);
