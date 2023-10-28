#include "JASDvd.hh"

JASTaskThread *JASDvd::CreateThread(s32 priority, s32 messageCount, u32 /* stackSize */) {
    return REPLACED(CreateThread)(priority, messageCount, 0x8000);
}
