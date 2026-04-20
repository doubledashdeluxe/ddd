#include "PauseManager.hh"

#include "game/GameAudioMain.hh"
#include "game/KartGamePad.hh"
#include "game/RaceMgr.hh"
#include "game/System.hh"
#include "game/WipeManager.hh"

#include <portable/Algorithm.hh>

bool PauseManager::paused() const {
    return m_paused;
}

JKRArchive *PauseManager::archive() const {
    return m_archive;
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

    if (m_isDemo || (!m_resultStart && !RaceMgr::Instance()->isReplay())) {
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
    if (m_hasPrintMemoryCard) {
        m_printMemoryCard->calc();
        s32 choice = m_printMemoryCard->getFinalChoice();
        if (choice == 0) {
            m_hasPrintMemoryCard = false;
            Pause2D::SetState(Pause2D::State::SlideOut);
        } else if (choice == 1) {
            m_hasPrintMemoryCard = false;
            m_printMemoryCard->ack();
            m_printMemoryCard->init(0x28);
        }
        return;
    }

    if (m_wipeOutFrame > 0 && m_wipeOutDuration > 0) {
        if (m_wipeOutFrame == m_wipeOutDuration) {
            System::GetDisplay()->startFadeOut(1);
        }
        m_wipeOutFrame++;
    }

    if (m_pauseEnd) {
        return;
    }

    const KartGamePad *pad = KartGamePad::GamePad(0);
    const JUTGamePad::CButton &button = pad->button();
    if (!m_paused) {
        if (!(button.risingEdge() & PAD_BUTTON_START)) {
            return;
        }

        s_pauseChoice = 13;
        m_paused = true;
        m_wasCanceled = false;
        m_pause2D->init();
        Pause2D::SetState(Pause2D::State::SlideIn);
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_PAUSE_ON);
    }

    m_pause2D->calc(pad);

    switch (Pause2D::State()) {
    case Pause2D::State::Reset:
        s_pauseChoice = m_wasCanceled ? 0 : Pause2D::Selector();
        if (s_pauseChoice == 0) {
            m_paused = false;
        }
        break;
    case Pause2D::State::Idle:
        if (button.risingEdge() & PAD_BUTTON_A) {
            switch (Pause2D::Selector()) {
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
        } else if (button.risingEdge() & PAD_BUTTON_START) {
            m_wasCanceled = true;
            Pause2D::SetState(Pause2D::State::SlideOut);
            GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_PAUSE_OFF);
        }
        break;
    }
}

PauseManager *PauseManager::Instance() {
    return s_instance;
}
