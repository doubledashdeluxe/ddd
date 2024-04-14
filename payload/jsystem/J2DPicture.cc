#include "J2DPicture.hh"

extern "C" {
#include <dolphin/GXTransform.h>
}

void J2DPicture::drawFullSet(f32 x, f32 y, f32 w, f32 h, Mtx34 *mtx) {
    x = m_box.start.x, y = m_box.start.y;
    s16 s0 = m_texCoords[0].x, t0 = m_texCoords[0].y;
    s16 s1 = m_texCoords[1].x, t1 = m_texCoords[1].y;
    s16 s2 = m_texCoords[2].x, t2 = m_texCoords[2].y;
    s16 s3 = m_texCoords[3].x, t3 = m_texCoords[3].y;

    u32 scissorX, scissorY, scissorWidth, scissorHeight;
    if (m_hasARScissor && m_hasARTrans && m_hasARScale) {
        GXGetScissor(&scissorX, &scissorY, &scissorWidth, &scissorHeight);

        u32 arScissorWidth = scissorWidth * s_arScale;
        u32 arScissorX = scissorX - (arScissorWidth - scissorWidth) / 2;
        GXSetScissor(arScissorX, scissorY, arScissorWidth, scissorHeight);
    }

    if (!m_hasARTrans || !m_hasARScale) {
        x += 0.5f * w;
    }

    if (!m_hasARTrans) {
        x += m_globalMtx[0][3];
        if (m_hasARShift) {
            if (m_hasARShiftRight || (x >= 0.5f * 608.0f && !m_hasARShiftLeft)) {
                x += s_arShift;
            } else {
                x -= s_arShift;
            }
        } else {
            x -= 0.5f * 608.0f;
            x /= s_arScale;
            x += 0.5f * 608.0f;
        }
        x -= m_globalMtx[0][3];
    }

    if (!m_hasARScale) {
        w /= s_arScale;
        if (m_hasARTexCoords) {
            s0 = s0 / s_arScale;
            s1 = s1 / s_arScale;
            s2 = s2 / s_arScale;
            s3 = s3 / s_arScale;
        }
    } else {
        if (!m_hasARTexCoords) {
            s0 = s0 * s_arScale;
            s1 = s1 * s_arScale;
            s2 = s2 * s_arScale;
            s3 = s3 * s_arScale;
        }
    }

    if (!m_hasARTrans || !m_hasARScale) {
        x -= 0.5f * w;
    }

    drawTexCoord(x, y, w, h, s0, t0, s1, t1, s2, t2, s3, t3, mtx);

    if (m_hasARScissor && m_hasARTrans && m_hasARScale) {
        GXSetScissor(scissorX, scissorY, scissorWidth, scissorHeight);
    }
}
