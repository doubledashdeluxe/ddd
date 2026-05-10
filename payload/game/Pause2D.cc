#include "Pause2D.hh"

#include "game/PauseManager.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <portable/Array.hh>

extern "C" {
#include <inttypes.h>
#include <stdio.h>
}

Pause2D::Pause2D(JKRHeap *heap, JKRArchive *archive)
    : m_hioNode(new (heap, 0) HioNode)
    , m_graphContext(System::GetJ2DOrtho())
    , m_screen(new (heap, 0) J2DScreen) {
    m_hioNode->isVisible = true;

    u32 count;
    switch (RaceInfo::Instance().getRaceMode()) {
    case RaceMode::TA:
        count = 5;
        m_selector = &Pause2D::selectorTA;
        m_setDraw = &Pause2D::setDrawTA;
        break;
    case RaceMode::VS:
        count = 5;
        m_selector = &Pause2D::selectorVS;
        m_setDraw = &Pause2D::setDrawVS;
        break;
    case RaceMode::GP:
    case RaceMode::Award:
    case RaceMode::StaffRoll:
        count = 2;
        m_selector = &Pause2D::selectorGP;
        m_setDraw = &Pause2D::setDrawGP;
        break;
    case RaceMode::Balloon:
    case RaceMode::Bomb:
    case RaceMode::Escape:
        if (SequenceInfo::Instance().m_isOnline) {
            count = 2;
            m_selector = &Pause2D::selectorGP;
            m_setDraw = &Pause2D::setDrawGP;
        } else {
            count = 6;
            m_selector = &Pause2D::selectorMG;
            m_setDraw = &Pause2D::setDrawMG;
        }
        break;
    }

    Array<char, 32> file;
    snprintf(file.values(), file.count(), "pause%" PRIu32 ".blo", count);
    m_screen->set(file.values(), 0x40000, archive);

    m_screen->search("pse_b")->setHasARScale(false, false);

    snprintf(file.values(), file.count(), "pause%" PRIu32 ".bpk", count);
    m_anmColors = J2DAnmLoaderDataBase::Load(file.values(), archive);
    m_screen->setAnimation(m_anmColors);
    snprintf(file.values(), file.count(), "pause%" PRIu32 ".bck", count);
    m_anmTransform = J2DAnmLoaderDataBase::Load(file.values(), archive);
    m_screen->setAnimation(m_anmTransform);

    for (u32 i = 0; i < count; i++) {
        Line &line = m_lines[i];
        line.anmTransform = J2DAnmLoaderDataBase::Load(file.values(), archive);
        line.pane = m_screen->search("Line%u", i + 1);
        line.pane->setAnimation(line.anmTransform);
    }
    for (u32 i = 0; i < count * 2; i++) {
        m_starPictures[i] = m_screen->search("kr_%c", 'a' + i)->downcast<J2DPicture>();
        m_starPositions[i] = m_starPictures[i]->m_offset;
        m_starPictures[i]->m_isVisible = false;
    }
    for (u32 i = 0; i < count; i++) {
        m_namePictures[i] = m_screen->search("PM%u", i + 1)->downcast<J2DPicture>();
    }
    if (count == 2) {
        m_namePictures[0]->changeTexture("Mozi_Continue.bti", 0);
        m_namePictures[1]->changeTexture("Mozi_Quit.bti", 0);
    } else if (count == 6) {
        m_namePictures[2]->changeTexture("Mozi_Change_Map.bti", 0);
        m_namePictures[4]->changeTexture("Mozi_Change_MiniGame.bti", 0);
    }
    m_foregroundPane = m_screen->search("kr");
    for (u32 i = 0; i < 2; i++) {
        m_cursorPictures[i] = m_screen->search("star%u", i + 1)->downcast<J2DPicture>();
    }

    if (count == 2) {
        setPadText();
    }

    init();
}

void Pause2D::draw() {
    if (PauseManager::Instance()->paused()) {
        m_graphContext->setViewport();
        m_screen->draw(0.0f, 0.0f, m_graphContext);
    } else {
        REPLACED(draw)();
    }
}

u32 Pause2D::GetState() {
    u32 state = REPLACED(GetState)();
    return state == State::Cooldown ? State::Idle : state;
}
