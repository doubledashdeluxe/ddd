#include "RaceApp.hh"

#include "game/SceneType.hh"

void RaceApp::ctrlRace() {
    REPLACED(ctrlRace)();

    if (m_nextScene == SceneType::PackSelect) {
        m_nextScene = SceneType::MapSelect;
    }
}
