#include "RaceMgr.hh"

bool RaceMgr::isReplay() const {
    return m_isReplay;
}

RaceMgr *RaceMgr::Instance() {
    return s_instance;
}
