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

    if (SequenceInfo::Instance().m_isOnline) {
        startOnline();
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

void LANNum2D::startOnline() {
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    if (onlineInfo.m_spectating) {
        return;
    }

    m_screen->search("NLan")->m_isVisible = true;
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 statusCount = raceInfo.getStatusCount();
    for (u32 i = 0; i < statusCount; i++) {
        if (raceInfo.isDemoKart(i)) {
            for (u32 j = 0; j < (statusCount <= 2 || i == 0 ? 2 : 1); j++) {
                m_screen->search("NP%u%u", i + 1, j + 1)->m_isVisible = false;
            }
        } else {
            for (u32 j = 0; j < onlineInfo.m_localKarts[i].playerCount; j++) {
                u32 colorIndex = onlineInfo.m_padIndices[i][j];
                J2DPicture::CornerColors cornerColors = Race2D::GetCornerColors(colorIndex);
                J2DPicture *boxPicture =
                        m_screen->search("PB%u%u", i + 1, j + 1)->downcast<J2DPicture>();
                boxPicture->m_cornerColors = cornerColors;
                J2DPicture *numPicture =
                        m_screen->search("PM%u%u", i + 1, j + 1)->downcast<J2DPicture>();
                numPicture->m_cornerColors = cornerColors;
                char name[32];
                snprintf(name, Count(name), "PlayerNumberSimple_%" PRIu32 "P.bti", colorIndex + 1);
                numPicture->changeTexture(name, 0);
            }
        }
        u32 kartIndex = J2DManager::StatusKart(i);
        u32 colorIndex = onlineInfo.colorIndex(kartIndex);
        J2DPicture::CornerColors cornerColors = Race2D::GetCornerColors(colorIndex);
        J2DPicture *boxPicture = m_screen->search("BNum%u", i + 1)->downcast<J2DPicture>();
        boxPicture->m_cornerColors = cornerColors;
        J2DPicture *numPicture = m_screen->search("KNum%u", i + 1)->downcast<J2DPicture>();
        numPicture->m_cornerColors = cornerColors;
    }
}

void LANNum2D::setText(const char *prefix, const char *text) {
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    kart2DCommon->changeUnicodeTexture(text, 30, *m_screen, prefix, true);
}
