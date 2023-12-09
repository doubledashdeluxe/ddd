#include "SceneMenu.hh"

#include "game/SystemRecord.hh"

#include <payload/CourseManager.hh>

void SceneMenu::init() {
    CourseManager::Instance()->unlock();

    REPLACED(init)();
}

void SceneMenu::reset() {
    SystemRecord::Instance().unlockAll();

    REPLACED(reset)();
}
