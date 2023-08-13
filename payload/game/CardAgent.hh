#pragma once

#include <common/Types.hh>

class CardAgent {
public:
    static void Ask(s32 command, s32 chan);

private:
    CardAgent();
};
static_assert(sizeof(CardAgent) == 0x1);
