#include "Race2D.hh"

void Race2D::setMinimapConfig(const MinimapConfig &minimapConfig) {
    m_minimapConfig = minimapConfig;
}

Race2D *Race2D::Instance() {
    return s_instance;
}
