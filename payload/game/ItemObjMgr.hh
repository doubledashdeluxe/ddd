#pragma once

#include "game/ItemObj.hh"
#include "game/ItemShuffleMgr.hh"

#include <jsystem/JSUList.hh>
#include <payload/Replace.hh>

class ItemObjMgr {
public:
    u32 usedCount(u32 item) const;
    u32 freeCount(u32 item) const;
    u32 moveCount(u32 item) const;
    ItemObj *getKartEquipItem(u32 kart, u32 driver) const;
    ItemObj *getKartEquipItem(u32 kart) const;
    void clearKartItemUseTriggerList();
    void REPLACED(startItemShuffleSingle)(u32 kart, bool r5);
    REPLACE void startItemShuffleSingle(u32 kart, bool r5);
    void REPLACED(startItemShuffleDouble)(u32 kart);
    REPLACE void startItemShuffleDouble(u32 kart);
    bool startItemShuffle(u32 kart, u32 character);
    bool stockItemToKart(u32 item, u32 kart, u32 character, bool isHeart, u8 r8);
    bool equipItemToKart(u32 item, u32 kart, u32 character, bool isHeart, u8 r8);

    static ItemObjMgr *Instance();

private:
    u32 getRobberyItemNum(u32 kart, u32 character);

    static bool CanShuffle(u32 kart);

    u8 _000[0x2cc - 0x000];
    ItemObj *m_equipItems[8][2];
    u8 _30c[0x35c - 0x30c];
    ItemShuffleMgr *m_shuffleMgrs[8][2];
    u8 _39c[0x3e8 - 0x39c];
    JSUList<ItemObj> m_usedLists[22];
    JSUList<ItemObj> m_freeLists[22];
    u8 _5f8[0x73c - 0x5f8];
    JSUList<ItemObj> m_moveLists[16];
    u8 _7fc[0x828 - 0x7fc];

    static ItemObjMgr *s_instance;
};
size_assert(ItemObjMgr, 0x828);
