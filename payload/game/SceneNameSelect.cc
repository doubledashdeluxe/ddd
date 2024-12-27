#include "SceneNameSelect.hh"

#include "game/CardAgent.hh"
#include "game/ExtendedSystemRecord.hh"
#include "game/GameAudioMain.hh"
#include "game/KartGamePad.hh"
#include "game/MenuTitleLine.hh"
#include "game/OnlineBackground.hh"
#include "game/OnlineInfo.hh"
#include "game/SceneFactory.hh"
#include "game/SequenceApp.hh"
#include "game/SequenceInfo.hh"
#include "game/SystemRecord.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <payload/online/ServerManager.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

SceneNameSelect::SceneNameSelect(JKRArchive *archive, JKRHeap *heap) : Scene(archive, heap) {
    SceneFactory *sceneFactory = SceneFactory::Instance();
    JKRArchive *lanEntryArchive = sceneFactory->archive(SceneFactory::ArchiveType::LanEntry);

    m_mainScreen.set("SelectName.blo", 0x1040000, lanEntryArchive);
    m_padCountScreen.set("PlayerIcon.blo", 0x1040000, m_archive);

    m_padCountScreen.search("Ns1234")->m_isVisible = false;
    m_padCountScreen.search("Ns12_3_4")->m_isVisible = false;
    m_padCountScreen.search("Ns12_34")->m_isVisible = false;

    m_mainAnmTransform = J2DAnmLoaderDataBase::Load("SelectName.bck", lanEntryArchive);
    m_mainScreen.search("N_Entry")->setAnimation(m_mainAnmTransform);
    m_nameAnmTransform = J2DAnmLoaderDataBase::Load("SelectName.bck", lanEntryArchive);
    for (u32 i = 0; i < 4; i++) {
        m_mainScreen.search("ENplay%u", i + 1)->setAnimation(m_nameAnmTransform);
    }
    m_nameCircleAnmTransform = J2DAnmLoaderDataBase::Load("SelectName.bck", lanEntryArchive);
    for (u32 i = 0; i < 4; i++) {
        m_mainScreen.search("Eplay%ub", i + 1)->setAnimation(m_nameCircleAnmTransform);
    }
    m_padCountAnmTransform = J2DAnmLoaderDataBase::Load("PlayerIcon.bck", m_archive);
    m_padCountScreen.search("N_Stok")->setAnimation(m_padCountAnmTransform);
    m_padCountCircleAnmTransform = J2DAnmLoaderDataBase::Load("PlayerIcon.bck", m_archive);
    m_padCountScreen.search("Cstok_pb")->setAnimation(m_padCountCircleAnmTransform);

    m_nameCircleAnmTransformFrame = 0;
    m_padCountCircleAnmTransformFrame = 0;

    for (u32 i = 0; i < 4; i++) {
        Array<J2DPane *, 3> panes;
        for (u32 j = 0; j < 3; j++) {
            panes[j] = m_mainScreen.search("Ename%u%u", i + 1, j + 1);
        }
        char *name = m_unsavedNames[i].values();
        KartGamePad *pad = KartGamePad::GamePad(i);
        m_nameEntryHelpers[i].reset(
                new (heap, 0x0) NameEntryHelper(panes.values(), name, heap, pad, i == 0));
    }
}

SceneNameSelect::~SceneNameSelect() {}

void SceneNameSelect::init() {
    if (SequenceApp::Instance()->prevScene() == SceneType::HowManyPlayers) {
        m_padCount = SequenceInfo::Instance().m_padCount;

        for (u32 i = 0; i < 4; i++) {
            m_mainScreen.search("ENplay%u", i + 1)->m_isVisible = i < m_padCount;
        }

        J2DPicture *picture = m_padCountScreen.search("Cstok_p")->downcast<J2DPicture>();
        Array<char, 32> name;
        snprintf(name.values(), name.count(), "Player%u.bti", m_padCount);
        picture->changeTexture(name.values(), 0);

        OnlineInfo &onlineInfo = OnlineInfo::Instance();
        if (!onlineInfo.m_hasNames) {
            readNames();
            onlineInfo.m_hasNames = true;
        }
        SequenceApp::Instance()->ready(SceneType::ServerSelect);

        for (u32 i = 0; i < m_nameEntryHelpers.count(); i++) {
            m_nameEntryHelpers[i]->init(m_unsavedNames[i].values());
        }
    }

    slideIn();
}

void SceneNameSelect::draw() {
    m_graphContext->setViewport();

    OnlineBackground::Instance()->draw(m_graphContext);
    MenuTitleLine::Instance()->draw(m_graphContext);

    m_mainScreen.draw(0.0f, 0.0f, m_graphContext);
    m_padCountScreen.draw(0.0f, 0.0f, m_graphContext);

    for (u32 i = 0; i < m_padCount; i++) {
        m_nameEntryHelpers[i]->draw(m_graphContext);
    }
}

void SceneNameSelect::calc() {
    (this->*m_state)();

    OnlineBackground::Instance()->calc();
    MenuTitleLine::Instance()->calc();

    m_nameAnmTransformFrame = m_padCount;
    m_nameCircleAnmTransformFrame = 14 + (m_nameCircleAnmTransformFrame - 13) % 61;
    m_padCountCircleAnmTransformFrame = 14 + (m_padCountCircleAnmTransformFrame - 13) % 61;

    m_mainAnmTransform->m_frame = m_mainAnmTransformFrame;
    m_nameAnmTransform->m_frame = m_nameAnmTransformFrame;
    m_nameCircleAnmTransform->m_frame = m_nameCircleAnmTransformFrame;
    m_padCountAnmTransform->m_frame = m_padCountAnmTransformFrame;
    m_padCountCircleAnmTransform->m_frame = m_padCountCircleAnmTransformFrame;

    m_mainScreen.animation();
    m_padCountScreen.animation();

    for (u32 i = 0; i < m_padCount; i++) {
        m_nameEntryHelpers[i]->calc();
    }
}

void SceneNameSelect::slideIn() {
    ServerManager::Instance()->unlock();
    MenuTitleLine::Instance()->drop("SelectName.bti");
    m_mainAnmTransformFrame = 0;
    m_padCountAnmTransformFrame = 0;
    m_state = &SceneNameSelect::stateSlideIn;
}

void SceneNameSelect::slideOut() {
    MenuTitleLine::Instance()->lift();
    m_state = &SceneNameSelect::stateSlideOut;
}

void SceneNameSelect::idle() {
    for (u32 i = 0; i < m_padCount; i++) {
        m_nameEntryHelpers[i]->startEdit();
    }
    m_state = &SceneNameSelect::stateIdle;
}

void SceneNameSelect::wait() {
    CardAgent::Ask(CardAgent::Command::WriteSystemFile, 0);
    m_state = &SceneNameSelect::stateWait;
}

void SceneNameSelect::nextScene() {
    m_state = &SceneNameSelect::stateNextScene;
}

void SceneNameSelect::stateSlideIn() {
    if (m_mainAnmTransformFrame < 15) {
        m_mainAnmTransformFrame++;
        if (m_mainAnmTransformFrame <= 9) {
            m_padCountAnmTransformFrame = m_mainAnmTransformFrame;
        }
    } else {
        idle();
    }
}

void SceneNameSelect::stateSlideOut() {
    if (m_mainAnmTransformFrame > 0) {
        m_mainAnmTransformFrame--;
        if (m_mainAnmTransformFrame <= 9) {
            m_padCountAnmTransformFrame = m_mainAnmTransformFrame;
        }
    } else {
        nextScene();
    }
}

void SceneNameSelect::stateIdle() {
    bool wasCanceled = false;
    for (u32 i = 0; i < m_padCount; i++) {
        if (m_nameEntryHelpers[i]->wasCanceled()) {
            wasCanceled = true;
            break;
        }
    }
    if (wasCanceled) {
        for (u32 i = 0; i < m_padCount; i++) {
            m_nameEntryHelpers[i]->stopEdit();
        }
        m_nextScene = SceneType::HowManyPlayers;
        slideOut();
        return;
    }

    bool wasSelected = true;
    for (u32 i = 0; i < m_padCount; i++) {
        if (!m_nameEntryHelpers[i]->wasSelected()) {
            wasSelected = false;
            break;
        }
    }
    if (wasSelected) {
        m_nextScene = m_padCount == 1 ? SceneType::ServerSelect : SceneType::TandemSelect;
        bool hasChanged = false;
        for (u32 i = 0; i < 4; i++) {
            if (strcmp(m_unsavedNames[i].values(), m_savedNames[i].values())) {
                hasChanged = true;
                break;
            }
        }
        if (hasChanged) {
            writeNames();
            wait();
        } else {
            slideOut();
        }
        return;
    }

    for (u32 i = 0; i < m_padCount; i++) {
        if (m_nameEntryHelpers[i]->wasSelected()) {
            const JUTGamePad::CButton &button = KartGamePad::GamePad(i)->button();
            if (button.risingEdge() & PAD_BUTTON_B) {
                m_nameEntryHelpers[i]->startEdit();
                GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_CANCEL_LITTLE);
            }
        }
    }
}

void SceneNameSelect::stateWait() {
    if (!CardAgent::IsReady()) {
        return;
    }

    CardAgent::Ack();
    slideOut();
}

void SceneNameSelect::stateNextScene() {
    if (!SequenceApp::Instance()->ready(m_nextScene)) {
        return;
    }

    SequenceApp::Instance()->setNextScene(m_nextScene);
}

void SceneNameSelect::readNames() {
    m_savedNames[0] = SystemRecord::Instance().m_name;
    ExtendedSystemRecord &extendedSystemRecord = ExtendedSystemRecord::Instance();
    for (u32 i = 0; i < extendedSystemRecord.m_names.count(); i++) {
        m_savedNames[1 + i] = extendedSystemRecord.m_names[i];
    }
    for (u32 i = 0; i < m_savedNames.count(); i++) {
        m_savedNames[i][m_savedNames[i].count() - 1] = '\0';
    }

    m_unsavedNames = m_savedNames;
}

void SceneNameSelect::writeNames() {
    SystemRecord::Instance().m_name = m_unsavedNames[0];
    ExtendedSystemRecord &extendedSystemRecord = ExtendedSystemRecord::Instance();
    for (u32 i = 0; i < extendedSystemRecord.m_names.count(); i++) {
        extendedSystemRecord.m_names[i] = m_unsavedNames[1 + i];
    }

    m_savedNames = m_unsavedNames;
}
