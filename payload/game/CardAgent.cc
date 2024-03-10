#include "CardAgent.hh"

bool CardAgent::IsReady() {
    return s_state == 0;
}

void CardAgent::Ack(u32 soundID) {
    s_printMemoryCard->ack(soundID);
}
