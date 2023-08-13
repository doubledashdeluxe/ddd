#pragma once

#include <common/Types.hh>

class ResMgr {
public:
    static void LoadKeepData();

private:
    ResMgr();
};
static_assert(sizeof(ResMgr) == 0x1);
