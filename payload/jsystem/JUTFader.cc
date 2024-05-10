#include "JUTFader.hh"

u32 JUTFader::getStatus() const {
    return m_status;
}

void JUTFader::setColor(u8 r, u8 g, u8 b) {
    m_color.r = r;
    m_color.g = g;
    m_color.b = b;
}
