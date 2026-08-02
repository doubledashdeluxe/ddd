#include "KartChecker.hh"

#include "game/SequenceInfo.hh"

u32 KartChecker::lapCount() const {
    return m_lapCount;
}

RaceTime &KartChecker::lapTime(u32 lap) const {
    return m_lapTimes[lap];
}

RaceTime &KartChecker::lapTotalTime(u32 lap) const {
    return m_lapTotalTimes[lap];
}

bool KartChecker::lapRenewal() const {
    return m_lapRenewal;
}

bool KartChecker::raceEnd() const {
    return m_raceEnd;
}

s32 KartChecker::lap() const {
    return m_lap;
}

const RaceTime &KartChecker::totalTime() const {
    return m_totalTime;
}

RaceTime &KartChecker::totalTime() {
    return m_totalTime;
}

s32 KartChecker::rank() const {
    return m_rank;
}

void KartChecker::checkLap(bool raceEnd) {
    REPLACED(checkLap)(raceEnd);

    if (!SequenceInfo::Instance().m_isOnline) {
        return;
    }

    if (m_checkLap && raceEnd && !m_raceEnd) {
        forceRaceEnd();
    }
}
