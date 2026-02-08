#include "ItemObjMgr.hh"

#include "game/KartCtrl.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceClient.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMode.hh"
#include "game/SequenceInfo.hh"

u32 ItemObjMgr::usedCount(u32 item) const {
    return m_usedLists[item].getNumLinks();
}

u32 ItemObjMgr::freeCount(u32 item) const {
    return m_freeLists[item].getNumLinks();
}

u32 ItemObjMgr::moveCount(u32 item) const {
    return m_moveLists[item].getNumLinks();
}

void ItemObjMgr::startItemShuffleSingle(u32 kart, bool r5) {
    if (!CanShuffle(kart)) {
        return;
    }

    RaceClient *raceClient = RaceClient::Instance();
    if (raceClient) {
        u8 driver = KartCtrl::Instance()->getKartBody(kart)->getDriver();
        if (startItemShuffle(kart, driver ^ 1)) {
            raceClient->setHasItem(kart, driver ^ 1);
        }
        return;
    }

    REPLACED(startItemShuffleSingle)(kart, r5);
}

void ItemObjMgr::startItemShuffleDouble(u32 kart) {
    if (!CanShuffle(kart)) {
        return;
    }

    RaceClient *raceClient = RaceClient::Instance();
    if (raceClient) {
        for (u32 i = 0; i < KartCharacterCount; i++) {
            if (startItemShuffle(kart, i)) {
                raceClient->setHasItem(kart, i);
            }
        }
        return;
    }

    REPLACED(startItemShuffleDouble)(kart);
}

bool ItemObjMgr::startItemShuffle(u32 kart, u32 character) {
    if (RaceInfo::Instance().getRaceMode() == RaceMode::Bomb) {
        if (getRobberyItemNum(kart, character) >= 5) {
            return false;
        }
    } else {
        if (getKartEquipItem(kart, character)) {
            return false;
        }
    }

    m_shuffleMgrs[kart][character]->start();
    return true;
}

ItemObjMgr *ItemObjMgr::Instance() {
    return s_instance;
}

bool ItemObjMgr::CanShuffle(u32 kart) {
    return !SequenceInfo::Instance().m_isOnline || OnlineInfo::Instance().m_karts[kart].local;
}
