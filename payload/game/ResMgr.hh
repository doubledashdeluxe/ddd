#pragma once

#include <common/Types.hh>

class ResMgr {
public:
    static void LoadKeepData();

private:
    ResMgr();
};
size_assert(ResMgr, 0x1);
