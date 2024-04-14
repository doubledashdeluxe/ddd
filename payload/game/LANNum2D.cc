#include "LANNum2D.hh"

#include "game/Kart2DCommon.hh"
#include "game/RaceInfo.hh"
#include "game/ResMgr.hh"

#include <jsystem/J2DPicture.hh>

extern "C" {
#include <stdio.h>
#include <string.h>
}

void LANNum2D::init() {
    s16 consoleCount = RaceInfo::Instance().getConsoleCount();
    for (s16 i = 1; i <= consoleCount; i++) {
        for (u32 j = 1; j <= (consoleCount <= 2 || i == 1 ? 2 : 1); j++) {
            m_screen->search("NP%u%u", i, j)->setHasARTrans(false, true);
        }
    }

    REPLACED(init)();
}

void LANNum2D::start() {
    REPLACED(start)();

    m_screen->search("NName")->m_isVisible = false;
    m_screen->search("NAuthor")->m_isVisible = false;

    const CourseManager::Course *course = ResMgr::GetCourse();
    if (!course || !course->name() || !course->author()) {
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
