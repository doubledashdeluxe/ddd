#include "LANNum2D.hh"

#include "game/J2DManager.hh"
#include "game/Kart2DCommon.hh"
#include "game/OnlineInfo.hh"
#include "game/Race2D.hh"
#include "game/RaceInfo.hh"
#include "game/ResMgr.hh"
#include "game/SequenceInfo.hh"

#include <jsystem/J2DPane.hh>

void LANNum2D::init() {
    u32 consoleCount = RaceInfo::Instance().getConsoleCount();
    for (u32 i = 1; i <= consoleCount; i++) {
        for (u32 j = 1; j <= (consoleCount <= 2 || i == 1 ? 2 : 1); j++) {
            m_screen->search("NP%u%u", i, j)->setHasARTrans(false, true);
        }
    }

    REPLACED(init)();
}

void LANNum2D::start() {
    REPLACED(start)();

    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    if (SequenceInfo::Instance().m_isOnline && !onlineInfo.m_spectating) {
        m_screen->search("NLan")->m_isVisible = true;
        u32 statusCount = RaceInfo::Instance().getStatusCount();
        for (u32 i = 0; i < statusCount; i++) {
            u32 kartIndex = J2DManager::StatusKart(i);
            u32 colorIndex = onlineInfo.colorIndex(kartIndex);
            J2DPicture::CornerColors cornerColors = Race2D::GetCornerColors(colorIndex);
            J2DPicture *boxPicture = m_screen->search("BNum%u", i + 1)->downcast<J2DPicture>();
            boxPicture->m_cornerColors = cornerColors;
            J2DPicture *numPicture = m_screen->search("KNum%u", i + 1)->downcast<J2DPicture>();
            numPicture->m_cornerColors = cornerColors;
        }
    }

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
    kart2DCommon->changeUnicodeTexture(text, 30, *m_screen, prefix, true);
}
