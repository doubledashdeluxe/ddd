#pragma once

#include "game/ItemObj.hh"

class ItemKinoko : public ItemObj {
public:
    void REPLACED(initFall)();
    REPLACE void initFall();

private:
    u8 _2b0[0x2b4 - 0x2b0];
};
size_assert(ItemKinoko, 0x2b4);
