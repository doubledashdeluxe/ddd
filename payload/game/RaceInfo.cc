#include "RaceInfo.hh"

u32 RaceInfo::getRaceLevel() const {
    return m_raceLevel;
}

RaceInfo &RaceInfo::Instance() {
    return s_instance;
}
