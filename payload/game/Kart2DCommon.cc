#include "Kart2DCommon.hh"

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

ResTIMG *Kart2DCommon::getNumberTexture(u32 index) {
    return m_numberTextures[index];
}

ResTIMG *Kart2DCommon::getCharacterIcon(u32 index) {
    return m_characterIcons[index];
}

ResTIMG *Kart2DCommon::getBattleIcon(u32 index) {
    return m_battleIcons[index];
}
