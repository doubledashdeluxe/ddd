#pragma once

#include "game/ItemObj.hh"

class ItemObjMgr {
public:
    ItemObj *getKartEquipItem(u32 kart, u32 driver);
    ItemObj *getKartEquipItem(u32 kart);
    void clearKartItemUseTriggerList();

    static ItemObjMgr *Instance();

private:
    u8 _000[0x828 - 0x000];

    static ItemObjMgr *s_instance;
};
size_assert(ItemObjMgr, 0x828);
