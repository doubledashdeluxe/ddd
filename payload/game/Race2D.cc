#include "Race2D.hh"

#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"

void Race2D::init() {
    m_raceScreen->setHasARTrans(false, true);
    RaceInfo &raceInfo = RaceInfo::Instance();
    s16 consoleCount = raceInfo.getConsoleCount();
    for (s16 i = 1; i <= consoleCount; i++) {
        m_raceScreen->search("itm%ubs", i)->setHasARShift(false, false);
        m_raceScreen->search("itm%ubu", i)->setHasARShift(false, false);
        m_raceScreen->search("itm%ufs", i)->setHasARShift(false, false);
        m_raceScreen->search("itm%ufu", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ub", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ubb", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ubh1", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ubh2", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%uf", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ufb", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ufh1", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ufh2", i)->setHasARShift(false, false);
    }
    if (consoleCount <= 2) {
        for (s16 i = 1; i <= consoleCount; i++) {
            for (u32 j = 0; j < 8; j++) {
                m_raceScreen->search("time%u%u", i, j)->setHasARShiftRight(true, false);
            }
            for (u32 j = 1; j <= 2; j++) {
                m_raceScreen->search("time%ut%u", i, j)->setHasARShiftRight(true, false);
            }
        }
    } else {
        m_raceScreen->search("course")->setHasARShift(false, false);
        m_maskScreen->setHasARTrans(false, true);
        m_maskScreen->setHasARShift(false, true);
        m_maskScreen->setHasARScale(false, true);
        m_maskScreen->setHasARTexCoords(false, true);
    }
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 8; j++) {
            m_playerNumberPictures[i][j]->setHasARTrans(false, false);
            m_playerNumberPictures[i][j]->setHasARShift(false, false);
        }
    }
    if (raceInfo.m_raceMode == RaceMode::Escape) {
        for (s16 i = 0; i < consoleCount; i++) {
            m_shineIndicationScreens[i]->setHasARTrans(false, true);
        }
    }
    for (s16 i = 0; i < consoleCount; i++) {
        for (u32 j = 0; j < 2; j++) {
            m_specialItemPictures[i][j]->setHasARTrans(false, false);
            m_specialItemPictures[i][j]->setHasARShift(false, false);
        }
    }

    REPLACED(init)();
}

void Race2D::setMinimapConfig(const MinimapConfig &minimapConfig) {
    m_minimapConfig = minimapConfig;
}

Race2D *Race2D::Instance() {
    return s_instance;
}

GXColor Race2D::GetPlayerNumberColor(u32 index) {
    return s_playerNumberColors[index];
}

void Race2D::getMapPos(s32 r4, const Vec3f &pos, Vec2f &mapPos) {
    REPLACED(getMapPos)(r4, pos, mapPos);

    s16 consoleCount = RaceInfo::Instance().getConsoleCount();
    if (consoleCount <= 2) {
        if (mapPos.x >= 0.5f * 608.0f) {
            mapPos.x += J2DPane::GetARShift();
        } else {
            mapPos.x -= J2DPane::GetARShift();
        }
    }
}

void Race2D::getItemInfo(s32 r4, s32 r5, s32 r6, f32 &x, f32 &y, f32 &scale) {
    REPLACED(getItemInfo)(r4, r5, r6, x, y, scale);

    if (x >= 0.5f * 608.0f) {
        x += J2DPane::GetARShift();
    } else {
        x -= J2DPane::GetARShift();
    }
    x -= 0.5f * 608.0f;
    x *= J2DPane::GetARScale();
    x += 0.5f * 608.0f;
}

void Race2D::getStartCharPos(s32 r4, s32 r5, f32 &f1) {
    REPLACED(getStartCharPos)(r4, r5, f1);

    if (f1 >= 0.5f * 608.0f) {
        f1 += J2DPane::GetARShift();
    } else {
        f1 -= J2DPane::GetARShift();
    }
}
