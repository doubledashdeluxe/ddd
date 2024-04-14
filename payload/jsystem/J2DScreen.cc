#include "J2DScreen.hh"

J2DPane *J2DScreen::search(const char *format, ...) {
    va_list vlist;
    va_start(vlist, format);
    J2DPane *pane = J2DPane::vsearch(format, vlist);
    va_end(vlist);
    return pane;
}

void J2DScreen::animationMaterials() {
    for (u32 i = 0; i < m_materialCount; i++) {
        m_materials[i].animation();
    }
}
