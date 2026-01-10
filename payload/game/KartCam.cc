#include "KartCam.hh"

#include "game/KartGamePad.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMgr.hh"
#include "game/SequenceInfo.hh"

extern "C" {
#include <math.h>
}

KartBody *KartCam::getBody() const {
    return m_body;
}

bool KartCam::hasJumped() const {
    return m_hasJumped;
}

void KartCam::init(bool isReset) {
    REPLACED(init)(isReset);

    m_isRearView = false;
    m_prevIsRearView = false;
    m_hasJumped = false;
}

void KartCam::setPtr() {
    bool isOnline = SequenceInfo::Instance().m_isOnline;
    if (isOnline && OnlineInfo::Instance().m_spectating) {
        m_mode = Mode::Land;
        m_flags |= 1 << 11;
        m_flags &= ~(1 << 8);
    }

    REPLACED(setPtr)();
}

void KartCam::doCameraMode() {
    m_hasJumped = m_isRearView != m_prevIsRearView;
    m_prevIsRearView = m_isRearView;

    bool isOnline = SequenceInfo::Instance().m_isOnline;
    if (isOnline && OnlineInfo::Instance().m_spectating && m_mode == Mode::Out) {
        u32 kartCount = RaceInfo::Instance().getKartCount();
        u32 targetKart = m_targetKart + kartCount;
        const JUTGamePad::CButton &button = KartGamePad::GamePad(0)->button();
        if (button.repeat() & (static_cast<u32>(JUTGamePad::PAD_MSTICK_LEFT) | PAD_TRIGGER_L)) {
            targetKart--;
        }
        if (button.repeat() & (static_cast<u32>(JUTGamePad::PAD_MSTICK_RIGHT) | PAD_TRIGGER_R)) {
            targetKart++;
        }
        targetKart %= kartCount;
        if (targetKart != m_targetKart) {
            RaceMgr::Instance()->console(0).changeTarget(targetKart, true);
            initOutView();
            m_hasJumped = true;
        }
    }

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
