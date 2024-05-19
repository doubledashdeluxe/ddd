#include "Kart2DCommon.hh"

ResTIMG *Kart2DCommon::getNumberTexture(u32 index) {
    return m_numberTextures[index];
}

ResTIMG *Kart2DCommon::getCharacterIcon(u32 index) {
    return m_characterIcons[index];
}

ResTIMG *Kart2DCommon::getBattleIcon(u32 index) {
    return m_battleIcons[index];
}
