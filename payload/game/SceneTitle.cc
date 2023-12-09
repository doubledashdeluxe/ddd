#include "SceneTitle.hh"

#include <payload/CourseManager.hh>

void SceneTitle::init() {
    CourseManager::Instance()->unlock();

    REPLACED(init)();
}
