#include "KartCam.hh"

extern "C" {
#include <math.h>
}

KartBody *KartCam::getBody() const {
    return m_body;
}

void KartCam::init(bool isReset) {
    REPLACED(init)(isReset);

    m_isRearView = false;
}

void KartCam::doCameraMode() {
    REPLACED(doCameraMode)();

    if (m_mode == Mode::Out && m_isRearView) {
        TVec3<f32> relPos = m_pos - m_target;
        m_pos.x -= 2.0f * relPos.x;
        m_pos.z -= 2.0f * relPos.z;
        f32 xzNorm = sqrt(relPos.x * relPos.x + relPos.z * relPos.z);
        m_pos.y -= 2.0f * (relPos.y - tan(m_relAngleY) * xzNorm);
        TVec3<f32> up(0.0f, 1.0f, 0.0f);
        C_MTXLookAt(m_mtx, &m_pos, &up, &m_target);
    }
}
