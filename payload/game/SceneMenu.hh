#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

class SceneMenu {
public:
    void REPLACED(reset)();
    REPLACE void reset();

private:
    u8 _0000[0x22C0 - 0x0000];
};
static_assert(sizeof(SceneMenu) == 0x22C0);
