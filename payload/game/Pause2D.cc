#include "Pause2D.hh"

void Pause2D::init() {
    m_screen->search("pse_b")->setHasARScale(false, false);

    REPLACED(init)();
}
