#include "RaceApp.hh"

#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/SceneType.hh"

void RaceApp::ctrlRace() {
    REPLACED(ctrlRace)();

    switch (m_nextScene) {
    case SceneType::CourseSelect:
        if (RaceInfo::Instance().m_raceMode == RaceMode::VS) {
            m_nextScene = SceneType::MapSelect;
        }
        break;
    case SceneType::PackSelect:
        m_nextScene = SceneType::MapSelect;
        break;
    }
}
