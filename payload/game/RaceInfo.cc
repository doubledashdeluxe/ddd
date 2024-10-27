#include "RaceInfo.hh"

#include "game/RaceMode.hh"

u32 RaceInfo::getRaceMode() const {
    return m_raceMode;
}

u32 RaceInfo::getRaceLevel() const {
    return m_raceLevel;
}

s16 RaceInfo::getKartCount() const {
    return m_kartCount;
}

s16 RaceInfo::getConsoleCount() const {
    return m_consoleCount;
}

const KartInfo &RaceInfo::getKartInfo(u32 index) const {
    return m_karts[index];
}

s16 RaceInfo::getAwardKartNo() const {
    return m_awardKartNo;
}

bool RaceInfo::isRace() const {
    switch (m_raceMode) {
    case RaceMode::TA:
    case RaceMode::GP:
    case RaceMode::VS:
        return true;
    default:
        return false;
    }
}

bool RaceInfo::isBattle() const {
    switch (m_raceMode) {
    case RaceMode::Balloon:
    case RaceMode::Robbery:
    case RaceMode::Bomb:
    case RaceMode::Escape:
        return true;
    default:
        return false;
    }
}

RaceInfo &RaceInfo::Instance() {
    return s_instance;
}
