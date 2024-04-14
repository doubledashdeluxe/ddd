#include "Goal2D.hh"

#include "game/RaceInfo.hh"
#include "game/System.hh"

#include <jsystem/J2DPane.hh>

void Goal2D::init() {
    s16 consoleCount = RaceInfo::Instance().getConsoleCount();
    for (s16 i = 0; i < consoleCount; i++) {
        u32 width;
        s32 left;
        switch (consoleCount) {
        case 1:
            width = System::Get3DVpW();
            left = System::Get3DVpX();
            break;
        case 2:
            width = System::Get3DVpDiv2W(i) * 2;
            left = System::Get3DVpDiv2X(i);
            break;
        default:
            width = System::Get3DVpDiv4W(i) * 2;
            left = System::Get3DVpDiv4X(i);
            break;
        }
        s32 mid = left + width / 2;
        width = width / J2DPane::GetARScale();
        TBox<f32> &box = m_orthoGraphs[i]->m_orthoBox;
        box.start.x = mid - static_cast<s32>(width / 2);
        box.end.x = mid + static_cast<s32>(width / 2);
    }

    REPLACED(init)();
}
