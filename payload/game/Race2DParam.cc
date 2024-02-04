#include "Race2DParam.hh"

#include "game/ResMgr.hh"

void Race2DParam::setX(f32 x) {
    const CourseManager::Course *course = ResMgr::GetCourse();
    if (course && course->isCustom()) {
        x = 0.0f;
    }

    m_x = x;
}

void Race2DParam::setY(f32 y) {
    const CourseManager::Course *course = ResMgr::GetCourse();
    if (course && course->isCustom()) {
        y = 0.0f;
    }

    m_y = y;
}

void Race2DParam::setS(f32 s) {
    const CourseManager::Course *course = ResMgr::GetCourse();
    if (course && course->isCustom()) {
        s = 1.0f;
    }

    m_s = s;
}
