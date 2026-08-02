#include "PauseManager.hh"

#include "game/GameAudioMain.hh"
#include "game/Goal2D.hh"
#include "game/KartCtrl.hh"
#include "game/KartGamePad.hh"
#include "game/NetGameMgr.hh"
#include "game/OnlineTimer.hh"
#include "game/PauseChoice.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMgr.hh"
#include "game/ResMgr.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"
#include "game/WipeManager.hh"

#include <portable/Algorithm.hh>

PauseManager::PauseManager(JKRHeap *heap)
    : m_isLAN(NetGameMgr::Instance()->isActive())
    , m_isOnline(SequenceInfo::Instance().m_isOnline)
    , m_archive(ResMgr::GetArchive(ResMgr::ArchiveID::MRAMLoc))
    , m_printMemoryCard(new (heap, 0) PrintMemoryCard(heap))
    , m_isDemo(true)
    , m_isVisible(false)
    , m_exec(false)
    , m_pause2D(new (heap, 0) Pause2D(heap, m_archive))
    , m_result2D(m_isOnline ? new (heap, 0) Result2D(heap, Result2D::Online())
                            : new (heap, 0) Result2D(heap))
    , m_graphContext(System::GetJ2DOrtho()) {
    u32 kartCount = RaceInfo::Instance().getKartCount();
    const KartCtrl *kartCtrl = KartCtrl::Instance();
    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    for (u32 i = 0; i < kartCount; i++) {
        for (u32 j = 0; j < 2; j++) {
            const KartGamePad *pad = kartCtrl->getKartGamePad(i, j);
            if (sequenceInfo.padPlayer(pad) != -1) {
                m_isDemo = false;
            }
        }
    }
    s_instance = this;
    reset();
}

bool PauseManager::paused() const {
    return m_paused;
}

void PauseManager::reset() {
    REPLACED(reset)();

    m_paused = false;
}

void PauseManager::draw() {
    if (!m_isVisible) {
        return;
    }

    if (!m_isLAN) {
        m_result2D->draw();
    }

    if (m_isDemo || (!m_resultStart && !RaceMgr::Instance()->isReplay()) ||
            SequenceInfo::Instance().m_isOnline) {
        m_pause2D->draw();
    }

    WipeManager *wipeManager = WipeManager::Instance();
    if (m_marioFrame) {
        switch (s_nextPauseChoice) {
        case 1:
        case 7:
            wipeManager->drawMario((m_marioFrame + 1) / 40.0f);
            break;
        default:
            m_marioFrame = 1; // Really?
            break;
        }
    }

    if (m_wipeOutFrame > 0 && m_wipeOutDuration > 0) {
        f32 t = static_cast<f32>(m_wipeOutFrame) / m_wipeOutDuration;
        t = Min(t, 1.0f);
        wipeManager->calcWipeCurtain(0, t);
        wipeManager->drawWipeCurtain(0, t);
    }

    if (m_hasPrintMemoryCard) {
        m_printMemoryCard->draw();
    }
}

void PauseManager::exec() {
    if (m_frame >= 0) {
        m_frame++;
    }

    if (m_hasPrintMemoryCard) {
        m_printMemoryCard->calc();
        s32 choice = m_printMemoryCard->getFinalChoice();
        if (choice == 0) {
            m_hasPrintMemoryCard = false;
            if (m_paused) {
                Pause2D::SetState(Pause2D::State::SlideOut);
            } else {
                m_resultEndFrame = 1;
                m_result2D->end();
            }
        } else if (choice == 1) {
            m_hasPrintMemoryCard = false;
            m_printMemoryCard->ack();
            m_printMemoryCard->init(0x28);
        }
        OnlineTimer::Instance()->calc();
        return;
    }

    if (m_wipeOutFrame > 0 && m_wipeOutDuration > 0) {
        if (m_wipeOutFrame == m_wipeOutDuration) {
            System::GetDisplay()->startFadeOut(1);
        }
        m_wipeOutFrame++;
    }

    const KartGamePad *pad = KartGamePad::GamePad(0);
    if (m_resultEndFrame > 0) {
        if (m_resultEndFrame < 11) {
            m_resultEndFrame++;
        } else {
            s_pauseChoice = Result2D::GetSelector();
        }
        m_result2D->calc(pad);
        return;
    }

    if (m_frame < 120) {
        return;
    }

    const JUTGamePad::CButton &button = pad->button();
    if (!m_paused && m_resultStart && Goal2D::End()) {
        m_result2D->calc(pad);

        if (!(button.risingEdge() & PAD_BUTTON_A) && !OnlineTimer::Instance()->hasExpired()) {
            return;
        }

        if (Result2D::GetState() != Result2D::State::Selector) {
            return;
        }

        if (!m_result2D->getAnmEnd()) {
            return;
        }

        switch (Result2D::GetSelector()) {
        case 8:
            m_hasPrintMemoryCard = true;
            m_printMemoryCard->init(0x27);
            m_printMemoryCard->calc();
            break;
        default:
            m_resultEndFrame = 1;
            m_result2D->end();
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_DECIDE);
            break;
        }
    }

    if (!m_paused && m_pauseEnd) {
        return;
    }

    if (!m_paused) {
        if (!(button.risingEdge() & PAD_BUTTON_START)) {
            return;
        }

        s_pauseChoice = PauseChoice::None;
        m_paused = true;
        m_wasCanceled = false;
        m_pause2D->init();
        Pause2D::SetState(Pause2D::State::SlideIn);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_PAUSE_ON);
    }

    m_pause2D->calc(pad);

    switch (Pause2D::GetState()) {
    case Pause2D::State::Reset:
        s_pauseChoice = m_wasCanceled ? 0 : Pause2D::GetSelector();
        if (s_pauseChoice == 0) {
            m_paused = false;
        }
        break;
    case Pause2D::State::Idle:
        if (button.risingEdge() & PAD_BUTTON_A) {
            switch (Pause2D::GetSelector()) {
            case 0:
                Pause2D::SetState(Pause2D::State::SlideOut);
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_PAUSE_OFF);
                break;
            case 8:
                m_hasPrintMemoryCard = true;
                m_printMemoryCard->init(0x27);
                m_printMemoryCard->calc();
                break;
            }
        } else if (button.risingEdge() & PAD_BUTTON_START || m_pauseEnd) {
            m_wasCanceled = true;
            Pause2D::SetState(Pause2D::State::SlideOut);
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_PAUSE_OFF);
        }
        break;
    }
}

void PauseManager::startResult() {
    m_result2D->start();
}

PauseManager *PauseManager::Instance() {
    return s_instance;
}

s32 PauseManager::PauseChoice() {
    return s_pauseChoice;
}
