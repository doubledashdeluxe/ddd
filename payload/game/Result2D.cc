#include "Result2D.hh"

void Result2D::init() {
    m_overScreen->search("OverBack")->setHasARScale(false, false);

    REPLACED(init)();
}
