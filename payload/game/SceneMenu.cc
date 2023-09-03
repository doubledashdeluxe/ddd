#include "SceneMenu.hh"

#include "game/SystemRecord.hh"

void SceneMenu::reset() {
    SystemRecord::Instance().unlockAll();

    REPLACED(reset)();
}
