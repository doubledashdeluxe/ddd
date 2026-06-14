#include "KartItem.hh"

#include "game/ItemObjMgr.hh"
#include "game/KartCtrl.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceClient.hh"

void KartItem::doTandemItemAnime() {
    RaceClient *raceClient = RaceClient::Instance();
    u32 index = m_body->getIndex();
    if (!raceClient || OnlineInfo::Instance().m_karts[index].local) {
        REPLACED(doTandemItemAnime)();
        return;
    }

    const RaceClient::ItemEvent *itemEvent;
    while (true) {
        itemEvent = raceClient->itemEvent(index);

        if (!itemEvent) {
            return;
        }

        if (itemEvent->frame + 5 <= MaxKartInputCount) {
            break;
        }

        raceClient->popItemEvent(index);
    }

    const KartCtrl *kartCtrl = KartCtrl::Instance();
    const KartAnime *kartAnime = kartCtrl->getKartAnime(index);
    if (kartAnime->isChangeStart() || m_body->isChange()) {
        return;
    }

    if (m_body->m_itemThrow) {
        return;
    }

    ItemObjMgr *itemObjMgr = ItemObjMgr::Instance();
    const ItemObj *item = itemObjMgr->getKartEquipItem(index);
    if (item) {
        if (item->getKind() != itemEvent->itemID && itemEvent->itemID != ItemID::None) {
            return;
        }
        u32 itemState = item->getState();
        switch (itemState) {
        case ItemObj::State::Divested:
        case ItemObj::State::HeartWaiting:
            return;
        }
    } else {
        u32 driver = m_body->getDriver();
        if (!itemObjMgr->equipItemToKart(itemEvent->itemID, index, driver ^ 1, false, 0)) {
            return;
        }
    }

    m_body->m_itemThrow = true;
    KartPad *kartPad = kartCtrl->getKartPad(index);
    kartPad->m_itemFrame = 5;
    kartPad->m_itemStickY = itemEvent->stickY * (1.0f / MaxStickY);
}

void KartItem::doTandemItemRelease() {
    RaceClient *raceClient = RaceClient::Instance();
    if (raceClient) {
        if (!m_body->m_itemThrow) {
            return;
        }

        const ItemObjMgr *itemObjMgr = ItemObjMgr::Instance();
        u32 index = m_body->getIndex();
        const ItemObj *item = itemObjMgr->getKartEquipItem(index);
        if (!item) {
            return;
        }

        u32 itemState = item->getState();
        switch (itemState) {
        case ItemObj::State::Divested:
        case ItemObj::State::HeartWaiting:
            return;
        }

        const KartCtrl *kartCtrl = KartCtrl::Instance();
        const KartPad *kartPad = kartCtrl->getKartPad(index);
        if (kartPad->m_itemFrame != 0) {
            return;
        }

        if (OnlineInfo::Instance().m_karts[index].local) {
            RaceClient::ItemEvent itemEvent;
            itemEvent.frame = 0;
            itemEvent.stickY = kartPad->m_itemStickY * MaxStickY;
            itemEvent.itemID = item->getKind();
            itemEvent.pos = item->m_pos;
            raceClient->pushItemEvent(index, itemEvent);
        } else {
            raceClient->popItemEvent(index);
        }
    }

    REPLACED(doTandemItemRelease)();
}
