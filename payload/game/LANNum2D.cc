#include "LANNum2D.hh"

#include "game/Kart2DCommon.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/SequenceInfo.hh"

#include <jsystem/J2DPicture.hh>
#include <payload/CourseManager.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

void LANNum2D::start() {
    REPLACED(start)();

    m_screen->search("NName")->m_isVisible = false;
    m_screen->search("NAuthor")->m_isVisible = false;

    const RaceInfo &raceInfo = RaceInfo::Instance();
    switch (raceInfo.getRaceMode()) {
    case RaceMode::VS:
    case RaceMode::Balloon:
    case RaceMode::Bomb:
    case RaceMode::Escape:
        break;
    default:
        return;
    }

    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    const CourseManager *courseManager = CourseManager::Instance();
    const CourseManager::Course *course;
    if (RaceInfo::Instance().isRace()) {
        course = &courseManager->raceCourse(sequenceInfo.m_packIndex, sequenceInfo.m_mapIndex);
    } else {
        course = &courseManager->battleCourse(sequenceInfo.m_packIndex, sequenceInfo.m_mapIndex);
    }
    if (!course->name() || !course->author()) {
        return;
    }

    m_screen->search("NName")->m_isVisible = true;
    m_screen->search("NAuthor")->m_isVisible = true;

    setText("Name", course->name());
    setText("Author", course->author());
}

void LANNum2D::start2() {
    REPLACED(start2)();

    m_screen->search("NName")->m_isVisible = false;
    m_screen->search("NAuthor")->m_isVisible = false;
}

void LANNum2D::setText(const char *prefix, const char *text) {
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    u32 length = strlen(text);
    f32 startX, endX;
    for (u32 i = 0; i < 30; i++) {
        Array<char, 9> tag;
        snprintf(tag.values(), tag.count(), "%s%u", prefix, i);
        J2DPicture *picture = m_screen->search(tag.values())->downcast<J2DPicture>();
        picture->m_isVisible = i < length;
        if (i < length) {
            char c = length > 30 && i >= 27 ? '.' : text[i];
            picture->changeTexture(kart2DCommon->getAsciiTexture(c), 0);
        }
        if (i == 0) {
            startX = picture->m_offset.x;
        }
        if (i == 0 || i < length) {
            endX = picture->m_offset.x;
        }
    }
    Array<char, 9> tag;
    snprintf(tag.values(), tag.count(), "%sO", prefix);
    J2DPane *pane = m_screen->search(tag.values());
    pane->m_offset.x = -(startX + endX) / 2.0f;
    pane->calcMtx();
}
