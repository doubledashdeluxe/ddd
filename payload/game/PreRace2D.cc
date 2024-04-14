#include "PreRace2D.hh"

#include "game/RaceInfo.hh"

void PreRace2D::init() {
    m_screen->setHasARTrans(false, true);
    RaceInfo &raceInfo = RaceInfo::Instance();
    for (u32 i = 0; i < (raceInfo.isBattle() ? 22 : 17); i++) {
        m_modePictures[i]->setHasARShiftLeft(true, false);
    }

    REPLACED(init)();
}
