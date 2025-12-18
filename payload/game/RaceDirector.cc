#include "RaceDirector.hh"

#include "game/RacePhase.hh"
#include "game/SequenceInfo.hh"

void RaceDirector::resetCommon() {
    m_race2DMode = Race2DMode::OneTwo;

    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    if (sequenceInfo.m_isOnline && m_racePhase == RacePhase::CourseDemo) {
        m_racePhase = RacePhase::NoCourseDemo;
    }

    REPLACED(resetCommon)();
}
