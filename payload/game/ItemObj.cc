#include "ItemObj.hh"

#include "game/ItemObjMgr.hh"
#include "game/KartCtrl.hh"
#include "game/KartPad.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceClient.hh"
#include "game/RaceInfo.hh"

u32 ItemObj::getState() const {
    return m_state;
}

const TVec3<f32> &ItemObj::getHandOffsetPos() {
    REPLACED(getHandOffsetPos)();
    updateHandOffsetPos();
    return m_handOffsetPos;
}

void ItemObj::updateHandOffsetPos() {
    const RaceClient *raceClient = RaceClient::Instance();
    if (!raceClient) {
        return;
    }

    u32 kartCount = RaceInfo::Instance().getKartCount();
    if (m_kartIndex >= kartCount) {
        return;
    }

    if (OnlineInfo::Instance().m_karts[m_kartIndex].local) {
        return;
    }

    const KartCtrl *kartCtrl = KartCtrl::Instance();
    const KartBody *kartBody = kartCtrl->getKartBody(m_kartIndex);
    if (!kartBody->m_itemThrow) {
        return;
    }

    if (ItemObjMgr::Instance()->getKartEquipItem(m_kartIndex) != this) {
        return;
    }

    const RaceClient::ItemEvent *itemEvent = raceClient->itemEvent(m_kartIndex);
    const KartPad *kartPad = kartCtrl->getKartPad(m_kartIndex);

    f32 t = (6.0f - kartPad->m_itemFrame) / 5;
    TVec3<f32> handOffsetPos;
    PSMTXMultVec(*m_handAnmMtx, &m_handOffsetPos, &handOffsetPos);
    TVec3<f32> posDiff;
    posDiff.x = itemEvent->pos.x - handOffsetPos.x;
    posDiff.y = 0.0f;
    posDiff.z = itemEvent->pos.z - handOffsetPos.z;
    handOffsetPos += t * posDiff;
    Mtx34 inv;
    PSMTXInverse(*m_handAnmMtx, inv);
    PSMTXMultVec(inv, &handOffsetPos, &m_handOffsetPos);
}
