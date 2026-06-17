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
    KartAnime *kartAnime = kartCtrl->getKartAnime(index);
    if (kartAnime->isChangeStart() || m_body->isChange()) {
        return;
    }

    if (m_body->m_itemThrow) {
        return;
    }

    ItemObjMgr *itemObjMgr = ItemObjMgr::Instance();
    ItemObj *item = itemObjMgr->getKartEquipItem(index);
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
    }

    u32 itemID = item ? item->getKind() : itemEvent->itemID;
    bool isTurtle = IsTurtle(itemID);
    bool isBanana = IsBanana(itemID);
    if ((isTurtle || isBanana) && kartAnime->isBackStart(index)) {
        return;
    }

    if (!item) {
        if (itemID == ItemID::None) {
            return;
        }

        u32 driver = m_body->getDriver();
        if (!itemObjMgr->equipItemToKart(itemID, index, driver ^ 1, false, 0)) {
            return;
        }

        item = itemObjMgr->getKartEquipItem(index);
    }

    m_body->m_itemThrow = true;
    KartPad *kartPad = kartCtrl->getKartPad(index);
    kartPad->m_itemFrame = 5;

    if (itemID == ItemID::GoldenMushroom) {
        if (!kartPad->m_hasGoldenMushroom) {
            kartPad->m_hasGoldenMushroom = true;
            kartPad->m_goldenMushroomFrame = 300;
            kartPad->m_goldenMushroom = item;
        }
        kartAnime->makeCrouchAnime();
        return;
    }

    bool isBack = isBanana || kartAnime->isBack(index);
    bool isItemBack = itemID != ItemID::Chomp;
    if (isTurtle || isBanana) {
        kartPad->m_itemStickY = itemEvent->stickY * (1.0f / MaxStickY);
        if (isBack) {
            isItemBack = kartPad->m_itemStickY <= 0.1f;
        } else {
            isItemBack = kartPad->m_itemStickY < -0.1f;
        }
        if (isItemBack) {
            m_body->m_itemBack = true;
        } else {
            m_body->m_itemFront = true;
        }
    }
    bool isSuccessionItem = isTurtle && item->isSuccessionItem();
    kartAnime->makeThrowAnime(isBack, isItemBack, isSuccessionItem);
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

bool KartItem::IsTurtle(u32 itemID) {
    switch (itemID) {
    case ItemID::GreenShell:
    case ItemID::BowserShell:
    case ItemID::RedShell:
    case ItemID::Bomb:
    case ItemID::MarioFireballs:
    case ItemID::YoshiEgg:
    case ItemID::BlueShell:
    case ItemID::TripleGreenShells:
    case ItemID::TripleRedShells:
    case ItemID::Bombs:
    case ItemID::Fireballs:
        return true;
    default:
        return false;
    }
}

bool KartItem::IsBanana(u32 itemID) {
    switch (itemID) {
    case ItemID::Banana:
    case ItemID::GiantBanana:
    case ItemID::Heart:
    case ItemID::FakeItemBox:
        return true;
    default:
        return false;
    }
}
