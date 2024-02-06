#include "LogoApp.hh"

#include "game/CardAgent.hh"
#include "game/GameAudioMain.hh"
#include "game/ResMgr.hh"
#include "game/SequenceApp.hh"
#include "game/System.hh"

#include <common/Log.hh>
#include <payload/CourseManager.hh>

void LogoApp::draw() {}

void LogoApp::calc() {
    switch (m_state) {
    case 0:
        CardAgent::Ask(1, 0);
        INFO("Loading bgm_0.aw...");
        break;
    case 1:
        if (!GameAudio::Main::Instance()->isWaveLoaded(5)) {
            return;
        }
        INFO(" done.\n");
        INFO("Loading se00_0.aw...");
        break;
    case 2:
        if (!GameAudio::Main::Instance()->isWaveLoaded(1)) {
            return;
        }
        INFO(" done.\n");
        break;
    case 3:
        ResMgr::LoadKeepData();
        break;
    case 4:
        if (!ResMgr::IsFinishedLoadingArc(ResMgr::ArchiveID::ARAM)) {
            return;
        }
        break;
    case 5:
        CourseManager::Instance()->start();
        break;
    case 6:
        SequenceApp::Call(SceneType::Title);
        return;
    }
    m_state++;
}

LogoApp *LogoApp::Create() {
    s_instance = new (System::GetAppHeap(), 0x4) LogoApp;
    s_instance->m_state = 0;
    return s_instance;
}
