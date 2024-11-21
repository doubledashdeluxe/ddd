#pragma once

#include <jsystem/J2DPicture.hh>
#include <jsystem/J2DScreen.hh>
#include <jsystem/JKRHeap.hh>
#include <payload/Replace.hh>

class Kart2DCommon {
public:
    Kart2DCommon();

    void changeUnicodeTexture(const char *text, u32 count, J2DScreen &screen, const char *prefix,
            f32 *startX = nullptr, f32 *endX = nullptr);
    void changeNumberTexture(s32 number, J2DPicture **pictures, u8 pictureCount, bool padWithZeros,
            bool fillWithHyphens);
    void changeNumberTexture(u32 number, u32 maxDigits, J2DScreen &screen, const char *prefix);
    ResTIMG *REPLACED(getAsciiTexture)(char c);
    REPLACE ResTIMG *getAsciiTexture(char c);
    ResTIMG *getUnicodeTexture(u32 cp);
    ResTIMG *getNumberTexture(u32 index);
    ResTIMG *getCharacterIcon(u32 index);
    ResTIMG *getBattleIcon(u32 index);

    static Kart2DCommon *Create(JKRHeap *heap);
    static Kart2DCommon *Instance();

private:
    static u32 CountDigits(u32 digits);

    u8 _000[0x004 - 0x000];
    ResTIMG *m_numberTextures[11];
    u8 _030[0x098 - 0x030];
    ResTIMG *m_characterIcons[20];
    u8 _0e8[0x198 - 0x0e8];
    ResTIMG *m_battleIcons[3];
    u8 _1a4[0x1bc - 0x1a4];

    static Kart2DCommon *s_instance;
};
size_assert(Kart2DCommon, 0x1bc);
