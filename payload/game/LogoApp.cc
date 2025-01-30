#include "LogoApp.hh"

#include "game/BBAMgr.hh"
#include "game/CardAgent.hh"
#include "game/GameAudioMain.hh"
#include "game/ResMgr.hh"
#include "game/SequenceApp.hh"
#include "game/System.hh"

#include <common/Log.hh>
#include <payload/CourseManager.hh>
#include <payload/online/Client.hh>
#include <payload/online/ServerManager.hh>

void LogoApp::draw() {}

void LogoApp::calc() {
    switch (m_state) {
    case 0:
        ResMgr::LoadKeepData();
        break;
    case 1:
        if (!ResMgr::IsFinishedLoadingArc(ResMgr::ArchiveID::MRAMLoc)) {
            return;
        }
        CardAgent::Ask(CardAgent::Command::CheckSystemFile, 0);
        System::StartAudio();
        INFO("Loading bgm_0.aw...");
        break;
    case 2:
        if (!GameAudio::Main::Instance()->isWaveLoaded(5)) {
            return;
        }
        INFO("Loaded bgm_0.aw.");
        INFO("Loading se00_0.aw...");
        break;
    case 3:
        if (!GameAudio::Main::Instance()->isWaveLoaded(1)) {
            return;
        }
        INFO("Loaded se00_0.aw.");
        CourseManager::Instance()->start();
        ServerManager::Instance()->start();
        Client::Init(System::GetAppHeap(), BBAMgr::Config());
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
