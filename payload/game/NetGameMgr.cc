#include "NetGameMgr.hh"

bool NetGameMgr::isActive() const {
    return m_state != 0;
}

NetGameMgr *NetGameMgr::Instance() {
    return s_instance;
}
