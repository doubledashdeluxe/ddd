#include "BombStart2D.hh"

void BombStart2D::init() {
    m_screen->setHasARTrans(false, true);

    REPLACED(init)();
}
