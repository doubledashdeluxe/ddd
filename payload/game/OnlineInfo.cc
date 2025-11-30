#include "OnlineInfo.hh"

u32 OnlineInfo::colorIndex(u32 kartIndex) const {
    return m_isFFA ? kartIndex : m_teams[kartIndex];
}

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
