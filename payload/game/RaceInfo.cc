#include "RaceInfo.hh"

#include "game/RaceMode.hh"

u32 RaceInfo::getRaceLevel() const {
    return m_raceLevel;
}

s16 RaceInfo::getKartCount() const {
    return m_kartCount;
}

s16 RaceInfo::getConsoleCount() const {
    return m_consoleCount;
}

s16 RaceInfo::getStatusCount() const {
    return m_statusCount;
}

const KartInfo &RaceInfo::getKartInfo(u32 index) const {
    return m_karts[index];
}

s16 RaceInfo::getAwardKartNo() const {
    return m_awardKartNo;
}

bool RaceInfo::isRace() const {
    return RaceMode::IsRace(m_raceMode);
}

bool RaceInfo::isBattle() const {
    return RaceMode::IsBattle(m_raceMode);
}

RaceInfo &RaceInfo::Instance() {
    return s_instance;
}
