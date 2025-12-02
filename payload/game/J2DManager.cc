#include "J2DManager.hh"

s32 J2DManager::goalAnmFrame() const {
    return m_goalAnmFrame;
}

J2DManager *J2DManager::Instance() {
    return s_instance;
}
