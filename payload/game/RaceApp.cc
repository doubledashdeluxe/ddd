#include "RaceApp.hh"

#include "game/ForceEffectMgr.hh"
#include "game/KartCtrl.hh"
#include "game/OnlineInfo.hh"
#include "game/OnlineTimer.hh"
#include "game/RaceClient.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/RacePhase.hh"
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
    } else if (drift > 0 && m_raceMgr->loopFrame() & 1 && !raceClient->hasResults()) {
        adjustment = -1;
    }
    if (adjustment >= 0) {
        u32 clientFrame = raceClient->clientFrame();
        adjustment = Min<s32>(adjustment, clientFrame - RaceClient::Frame() + 30 - 1);
    }
    if (adjustment >= 0 && !raceClient->hasResults()) {
        u32 serverFrame = raceClient->serverFrame();
        adjustment = Min<s32>(adjustment, serverFrame - RaceClient::Frame() + 30 - 1);
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

    switch (m_raceMgr->raceDirector()->racePhase()) {
    case RacePhase::PersonalRoom:
        m_nextScene = SceneType::PersonalRoom;
        m_state = 3;
        break;
    case RacePhase::PlayerList:
        m_nextScene = SceneType::PlayerList;
        m_state = 3;
        break;
    }
}

void RaceApp::ctrlRecorder() {
    switch (m_recPhase) {
    case RecPhase::Waiting:
        if (m_raceMgr->isJugemCountStart()) {
            m_recPhase = RecPhase::Ready;
        }
        break;
    case RecPhase::Ready:
        m_recPhase = RecPhase::Running;
        break;
    }

    if (m_recPhase == RecPhase::Ready) {
        switch (m_recState) {
        case RecState::Recording:
            m_recorder->rec();
            break;
        case RecState::Playing:
            m_recorder->play();
            break;
        }
    }

    if (m_recPhase != RecPhase::Running || m_recState != RecState::Recording) {
        return;
    }

    m_recorder->nextFrame();

    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const KartCtrl *kartCtrl = KartCtrl::Instance();
    for (u32 i = 0; i < onlineInfo.m_localKartCount; i++) {
        if (m_recGoalFlags & 1 << i) {
            continue;
        }

        u32 kartIndex = onlineInfo.m_localKartIndices[i];
        if (!m_raceMgr->kartChecker(kartIndex)->raceEnd()) {
            continue;
        }

        const RaceTime &totalTime = m_raceMgr->kartChecker(kartIndex)->totalTime();
        for (u32 j = 0; j < onlineInfo.m_karts[i].playerCount; j++) {
            KartGamePad *pad = kartCtrl->getKartGamePad(kartIndex, j);
            s32 port = pad->padPort();
            m_recorder->finalizeRecord(port, totalTime);
            m_recorder->setRecord(port, nullptr);
        }

        m_recGoalFlags |= 1 << i;
    }

    u32 mask = (1 << onlineInfo.m_localKartCount) - 1;
    if ((m_recGoalFlags & mask) != mask) {
        return;
    }

    m_recorder->stop();
    m_recState = RecState::Stopped;
}
