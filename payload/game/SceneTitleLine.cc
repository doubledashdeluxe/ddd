#include "SceneTitleLine.hh"

void SceneTitleLine::init(s32 index) {
    m_screen->search("T_line")->setHasARScale(false, false);

    REPLACED(init)(index);
}
