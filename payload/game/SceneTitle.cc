#include "SceneTitle.hh"

#include <common/Console.hh>
#include <payload/CourseManager.hh>

void SceneTitle::init() {
    CourseManager::Instance()->unlock();

    Console::Instance()->m_isActive = false;

    REPLACED(init)();
}
