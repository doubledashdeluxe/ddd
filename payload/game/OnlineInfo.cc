#include "OnlineInfo.hh"

void OnlineInfo::reset() {
    m_hasIDs = false;
}

OnlineInfo &OnlineInfo::Instance() {
    return s_instance;
}

OnlineInfo::OnlineInfo() : m_roomCounter(0) {
    reset();
}

OnlineInfo OnlineInfo::s_instance;
