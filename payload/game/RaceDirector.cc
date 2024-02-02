#include "RaceDirector.hh"

void RaceDirector::resetCommon() {
    m_race2DMode = Race2DMode::OneTwo;

    REPLACED(resetCommon)();
}
