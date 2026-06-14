#include "RaceApp.hh"

#include "game/ForceEffectMgr.hh"
#include "game/OnlineTimer.hh"
#include "game/RaceClient.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/SceneType.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

RaceApp *RaceApp::Create() {
    if (!s_instance) {
        s_instance = new (System::GetAppHeap(), 0x4) RaceApp;
        if (SequenceInfo::Instance().m_isOnline) {
            RaceClient::Create();
        }
    }
    return s_instance;
}

RaceApp *RaceApp::Instance() {
    return s_instance;
}

RaceApp::~RaceApp() {
    OnlineTimer::Destroy();
    if (SequenceInfo::Instance().m_isOnline) {
        RaceClient::Destroy();
    }
    ForceEffectMgr::Destroy();
    PadMgr::SetRecorder(nullptr);
    s_instance = nullptr;
}

void RaceApp::calc() {
    RaceClient *raceClient = RaceClient::Instance();
    if (!raceClient) {
        REPLACED(calc)();
        return;
    }

    if (m_state != 2) {
        REPLACED(calc)();
        return;
    }

    raceClient->read();

    s32 adjustment = 0;
    s32 drift = raceClient->drift();
    if (drift <= -120) {
        adjustment = -drift;
    } else if (drift < 0) {
        adjustment = 1;
    } else if (drift > 0 && m_raceMgr->loopFrame() & 1) {
        adjustment = -1;
    }
    if (adjustment >= 0) {
        u32 clientFrame = raceClient->clientFrame() - MinClientFrame;
        adjustment = Min<s32>(adjustment, clientFrame - m_raceMgr->frame() + 30 - 1);
    }
    raceClient->adjustDrift(adjustment);
    do {
        if (adjustment >= 0) {
            raceClient->calcBefore();
        }
        m_raceMgr->calcRace(adjustment);
        ctrlRecorder();
        if (adjustment >= 0) {
            m_raceMgr->updateRace();
        }
        if (adjustment <= 0) {
            ctrlRace();
        }
        if (adjustment >= 0) {
            raceClient->calcAfter();
        }
    } while (adjustment-- > 0);

    raceClient->write();
}

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
