#include "J2DScreen.hh"

J2DPane *J2DScreen::search(const char *tag) {
    return J2DPane::search(tag);
}

void J2DScreen::animationMaterials() {
    for (u32 i = 0; i < m_materialCount; i++) {
        m_materials[i].animation();
    }
}
