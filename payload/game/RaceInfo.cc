#include "RaceInfo.hh"

u32 RaceInfo::getRaceMode() const {
    return m_raceMode;
}

u32 RaceInfo::getRaceLevel() const {
    return m_raceLevel;
}

s16 RaceInfo::getAwardKartNo() const {
    return m_awardKartNo;
}

RaceInfo &RaceInfo::Instance() {
    return s_instance;
}
