#include "RaceDirector.hh"

#include "game/RaceClient.hh"
#include "game/RacePhase.hh"
#include "game/SequenceInfo.hh"

u32 RaceDirector::racePhase() const {
    return m_racePhase;
}

bool RaceDirector::isFrameRenewal() const {
    return m_isFrameRenewal;
}

void RaceDirector::resetCommon() {
    m_race2DMode = Race2DMode::OneTwo;

    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    if (sequenceInfo.m_isOnline && m_racePhase == RacePhase::CourseDemo) {
        m_racePhase = RacePhase::NoCourseDemo;
    }

    REPLACED(resetCommon)();
}

void RaceDirector::doRunning(bool r4) {
    RaceClient *raceClient = RaceClient::Instance();
    if (raceClient && !raceClient->ok()) {
        setPhaseWait(RacePhase::Title, true, true, 35);
    }

    REPLACED(doRunning)(r4);
}
