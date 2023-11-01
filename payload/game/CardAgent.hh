#pragma once

#include <common/Types.hh>

class CardAgent {
public:
    static void Ask(s32 command, s32 chan);

private:
    CardAgent();
};
size_assert(CardAgent, 0x1);
