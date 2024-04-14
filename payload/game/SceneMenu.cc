#include "SceneMenu.hh"

#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/SceneType.hh"
#include "game/SystemRecord.hh"

#include <payload/CourseManager.hh>

void SceneMenu::init() {
    CourseManager::Instance()->unlock();

    m_selectModeScreen->search("MtaName")->setHasARScissor(true, true);
    m_selectModeScreen->search("N_Pw_gp1")->setHasARScissor(true, true);
    m_selectModeScreen->search("N_Pw_gp2")->setHasARScissor(true, true);
    m_selectModeScreen->search("N_Pw_vs")->setHasARScissor(true, true);

    REPLACED(init)();
}

void SceneMenu::reset() {
    SystemRecord::Instance().unlockAll();

    REPLACED(reset)();
}

void SceneMenu::setRaceData() {
    REPLACED(setRaceData)();

    if (RaceInfo::Instance().getRaceMode() == RaceMode::VS) {
        m_nextScene = SceneType::PackSelect;
    }
}
