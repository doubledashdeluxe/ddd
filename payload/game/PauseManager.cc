#include "PauseManager.hh"

#include "game/RaceMgr.hh"
#include "game/WipeManager.hh"

#include <portable/Algorithm.hh>

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
