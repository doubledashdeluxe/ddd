#include "Kart2DCommon.hh"

#include "game/ResMgr.hh"

extern "C" {
#include <string.h>
}

void Kart2DCommon::changeAsciiTexture(const char *text, u32 count, J2DScreen &screen,
        const char *prefix, f32 *startX, f32 *endX) {
    u32 length = strlen(text);
    for (u32 i = 0; i < count; i++) {
        J2DPicture *picture = screen.search("%s%u", prefix, i)->downcast<J2DPicture>();
        picture->m_isVisible = i < length;
        if (i < length) {
            char c = length > count && i + 3 >= count ? '.' : text[i];
            picture->changeTexture(getAsciiTexture(c), 0);
        }
        if (i == 0 && startX) {
            *startX = picture->m_offset.x;
        }
        if ((i == 0 || i < length) && endX) {
            *endX = picture->m_offset.x;
        }
    }
}

void Kart2DCommon::changeNumberTexture(u32 number, u32 maxDigits, J2DScreen &screen,
        const char *prefix) {
    u32 digits = CountDigits(number);
    if (digits > maxDigits) {
        number = 999999999;
        digits = maxDigits;
    }
    for (u32 i = 1; i <= maxDigits; i++) {
        for (u32 j = 0; j < i; j++) {
            J2DPicture *picture = screen.search("%s%u%u", prefix, i, j)->downcast<J2DPicture>();
            picture->m_isVisible = i == digits;
        }
    }
    for (u32 i = 0; i < digits; i++) {
        J2DPicture *picture = screen.search("%s%u%u", prefix, digits, i)->downcast<J2DPicture>();
        picture->changeTexture(getNumberTexture(number % 10), 0);
        number /= 10;
    }
}

ResTIMG *Kart2DCommon::getAsciiTexture(char c) {
    const char *path;
    switch (c) {
    case '\'':
        path = "timg/MarioFontApostrophe.bti";
        break;
    case '/':
        path = "timg/MarioFontSlash.bti";
        break;
    default:
        return REPLACED(getAsciiTexture(c));
    }
    return reinterpret_cast<ResTIMG *>(ResMgr::GetPtr(ResMgr::ArchiveID::Race2D, path));
}

ResTIMG *Kart2DCommon::getNumberTexture(u32 index) {
    return m_numberTextures[index];
}

ResTIMG *Kart2DCommon::getCharacterIcon(u32 index) {
    return m_characterIcons[index];
}

ResTIMG *Kart2DCommon::getBattleIcon(u32 index) {
    return m_battleIcons[index];
}

u32 Kart2DCommon::CountDigits(u32 number) {
    u32 digits = 0;
    do {
        digits++;
        number /= 10;
    } while (number > 0);
    return digits;
}
