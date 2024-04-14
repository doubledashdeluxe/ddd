#include "SceneCourseSelect.hh"

void SceneCourseSelect::init() {
    m_underScreen->search("N_Remove")->setHasARScissor(true, true);
    m_underScreen->search("Cwhite01")->setHasARScissor(true, false);
    m_underScreen->search("Cwhite02")->setHasARScissor(true, false);

    REPLACED(init)();
}
