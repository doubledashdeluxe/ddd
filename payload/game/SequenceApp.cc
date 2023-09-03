#include "SequenceApp.hh"

#include "game/SystemRecord.hh"

void SequenceApp::Call(s32 sceneType) {
    if (sceneType == SceneType::Title) {
        // Alternate title screen
        SystemRecord::Instance().setGameFlag(SystemRecord::GameFlag::All);
    }

    REPLACED(Call)(sceneType);
}
