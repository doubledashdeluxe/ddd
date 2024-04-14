#include "SceneRecord.hh"

void SceneRecord::init() {
    m_backgroundScreen->search("Back")->setHasARScale(false, false);
    m_backgroundScreen->search("B_mozi")->setHasARScale(false, false);

    REPLACED(init)();
}
