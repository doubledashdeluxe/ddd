#include "GhostScreen.hh"

#include "game/RaceInfo.hh"
#include "game/System.hh"

extern "C" {
#include <dolphin/GXTexture.h>
}

GhostScreen::GhostScreen()
    : m_width(Width())
    , m_height(Height())
    , m_ready(false)
    , m_sceneTexture(m_width, m_height, GX_TF_RGB565)
    , m_ghostTexture(m_width, m_height, GX_TF_I8) {
    reset();
}

void GhostScreen::setConsole(u32 index) {
    m_x = X(index);
    m_y = Y(index);
}

bool GhostScreen::hidden() const {
    return m_hidden;
}

u32 GhostScreen::X(u32 index) {
    u32 consoleCount = RaceInfo::Instance().getConsoleCount();
    switch (consoleCount) {
    case 1:
        return System::Get3DScisX();
    case 2:
        return System::Get3DScisDiv2X(index);
    default:
        return System::Get3DScisDiv4X(index);
    }
}

u32 GhostScreen::Y(u32 index) {
    u32 consoleCount = RaceInfo::Instance().getConsoleCount();
    switch (consoleCount) {
    case 1:
        return System::Get3DScisY();
    case 2:
        return System::Get3DScisDiv2Y(index);
    default:
        return System::Get3DScisDiv4Y(index);
    }
}

u32 GhostScreen::Width() {
    u32 consoleCount = RaceInfo::Instance().getConsoleCount();
    switch (consoleCount) {
    case 1:
        return System::Get3DScisW();
    case 2:
        return System::Get3DScisDiv2W(0);
    default:
        return System::Get3DScisDiv4W(0);
    }
}

u32 GhostScreen::Height() {
    u32 consoleCount = RaceInfo::Instance().getConsoleCount();
    switch (consoleCount) {
    case 1:
        return System::Get3DScisH();
    case 2:
        return System::Get3DScisDiv2H(0);
    default:
        return System::Get3DScisDiv4H(0);
    }
}
