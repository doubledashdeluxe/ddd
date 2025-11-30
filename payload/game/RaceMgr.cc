#include "RaceMgr.hh"

RaceDrawer *RaceMgr::raceDrawer() {
    return m_raceDrawer;
}

bool RaceMgr::isReplay() const {
    return m_isReplay;
}

KartLoader *RaceMgr::kartLoader(u32 index) {
    return m_kartLoaders[index];
}

RaceMgr *RaceMgr::Instance() {
    return s_instance;
}
