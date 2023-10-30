#include "System.hh"

#include "game/CardAgent.hh"
#include "game/GameAudioMain.hh"
#include "game/ResMgr.hh"
#include "game/SequenceApp.hh"

extern "C" {
#include <dolphin/VI.h>
}

void System::Init() {
    REPLACED(Init)();
    s_loadTask->request(REPLACED(StartAudio), nullptr, nullptr);

    CardAgent::Ask(1, 0);

    while (!GameAudio::Main::Instance()->isWaveLoaded(1)) {
        VIWaitForRetrace();
    }
    while (!GameAudio::Main::Instance()->isWaveLoaded(5)) {
        VIWaitForRetrace();
    }

    ResMgr::LoadKeepData();

    SequenceApp::Call(SceneType::Title);
}

void System::StartAudio(void * /* userData */) {}
