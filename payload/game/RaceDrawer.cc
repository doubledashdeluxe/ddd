#include "RaceDrawer.hh"

KartDrawer *RaceDrawer::kartDrawer(u32 index) {
    return m_kartDrawers + index;
}

ZCaptureMgr *RaceDrawer::zCaptureMgr() {
    return m_zCaptureMgr;
}
