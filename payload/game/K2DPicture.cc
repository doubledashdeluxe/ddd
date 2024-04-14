#include "K2DPicture.hh"

void K2DPicture::drawK2D(f32 x, f32 y, f32 w, f32 h, bool r4) {
    if (!m_hasARTrans) {
        if (m_hasARShift) {
            if (m_hasARShiftRight || (x >= 0.5f * 608.0f && !m_hasARShiftLeft)) {
                x += s_arShift;
            } else {
                x -= s_arShift;
            }
        } else {
            x += 0.5f * w;
            x -= 0.5f * 608.0f;
            x /= s_arScale;
            x += 0.5f * 608.0f;
            x -= 0.5f * w;
        }
    }

    REPLACED(drawK2D)(x, y, w, h, r4);
}
