#include "ItemShuffleMgr.hh"

#include "game/ItemObjMgr.hh"
#include "game/RaceClient.hh"
#include "game/RaceInfo.hh"

bool ItemShuffleMgr::doShuffle(u32 kartIndex, u32 characterIndex) {
    u16 frame = m_frame;
    bool hasItem = REPLACED(doShuffle)(kartIndex, characterIndex);

    RaceClient *raceClient = RaceClient::Instance();
    if (!hasItem || !raceClient || raceClient->hasItem(kartIndex, characterIndex)) {
        return hasItem;
    }

    m_frame = frame;
    m_flags = 1 << 0;
    return false;
}

u32 ItemShuffleMgr::slotItem(u32 kartIndex, u32 characterIndex) {
    RaceClient *raceClient = RaceClient::Instance();
    if (!raceClient) {
        return REPLACED(slotItem)(kartIndex, characterIndex);
    }

    u32 item = raceClient->itemID(kartIndex, characterIndex);
    if (RaceInfo::Instance().isRace()) {
        ItemObjMgr::Instance()->stockItemToKart(item, kartIndex, characterIndex, false, 0);
    }
    return item;
}

void ItemShuffleMgr::start() {
    m_flags |= 1 << 0;
    m_isSpecialSlot = false;
}
