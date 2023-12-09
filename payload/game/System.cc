#include "System.hh"

#include "game/CardAgent.hh"
#include "game/GameAudioMain.hh"
#include "game/ResMgr.hh"
#include "game/SequenceApp.hh"

extern "C" {
#include <dolphin/VI.h>
}
#include <payload/CourseManager.hh>

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
    while (!ResMgr::IsFinishedLoadingArc(ResMgr::ArchiveId::MRAM)) {
        VIWaitForRetrace();
    }

    CourseManager::Instance()->start();

    SequenceApp::Call(SceneType::Title);
}

JFWDisplay *System::GetDisplay() {
    return s_display;
}

JKRHeap *System::GetAppHeap() {
    return s_appHeap;
}

JKRTask *System::GetLoadTask() {
    return s_loadTask;
}

void System::StartAudio(void * /* userData */) {}
