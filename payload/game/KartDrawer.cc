#include "KartDrawer.hh"

bool KartDrawer::isVisible() const {
    return !m_hiddenFlags;
}
