#pragma once

#include <common/Types.hh>

class BBAMgr {
public:
    u32 getState();

    static BBAMgr *Instance();

private:
    u8 _000[0x9c0 - 0x000];

    static BBAMgr *s_instance;
};
size_assert(BBAMgr, 0x9c0);
