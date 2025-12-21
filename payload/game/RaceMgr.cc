#include "RaceMgr.hh"

#include "game/ItemObjMgr.hh"
#include "game/J2DManager.hh"
#include "game/MotorManager.hh"

RaceDirector *RaceMgr::raceDirector() const {
    return m_raceDirector;
}

RaceDrawer *RaceMgr::raceDrawer() const {
    return m_raceDrawer;
}

u8 RaceMgr::loopFrame() const {
    return m_loopFrame;
}

u32 RaceMgr::frame() const {
    return m_frame;
}

bool RaceMgr::isReplay() const {
    return m_isReplay;
}

KartChecker *RaceMgr::kartChecker(u32 index) const {
    return m_kartCheckers[index];
}

KartLoader *RaceMgr::kartLoader(u32 index) const {
    return m_kartLoaders[index];
}

void RaceMgr::calcRace(s32 adjustment) {
    if (adjustment <= 0) {
        m_raceDirector->calc();
        MotorManager::Instance()->exec();
        J2DManager::Instance()->calc();
        ItemObjMgr::Instance()->clearKartItemUseTriggerList();
        if (m_award2D) {
            m_award2D->calc();
        }
        if (m_staffRoll2D) {
            m_staffRoll2D->calc();
        }
    }
    if (adjustment >= 0) {
        framework();
        m_frame++;
    }
    checkRank();
    if (adjustment <= 0) {
        m_loopFrame++;
    }
}

RaceMgr *RaceMgr::Instance() {
    return s_instance;
}
