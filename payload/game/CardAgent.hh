#pragma once

#include "game/PrintMemoryCard.hh"

class CardAgent {
public:
    static void Ask(s32 command, s32 chan);
    static bool IsReady();
    static void Ack(u32 soundID);

private:
    CardAgent();

    static u32 s_state;
    static PrintMemoryCard *s_printMemoryCard;
};
size_assert(CardAgent, 0x1);
