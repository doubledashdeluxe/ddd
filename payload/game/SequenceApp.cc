#include "SequenceApp.hh"

#include "game/SystemRecord.hh"

void SequenceApp::Call(s32 sceneType) {
    if (sceneType == SceneType::Title) {
        // Alternate title screen
        SystemRecord::Instance().setGameFlag(SystemRecord::GameFlag::All);
    }

    REPLACED(Call)(sceneType);
}

void SequenceApp::setNextScene(s32 sceneType) {
    s_nextScene = sceneType;
    m_state = 0;
}

SequenceApp *SequenceApp::Instance() {
    return s_instance;
}
