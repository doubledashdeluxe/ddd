#pragma once

#include <common/Types.hh>

class CardMgr {
public:
    static void Probe();

private:
    CardMgr();
};
size_assert(CardMgr, 0x1);
