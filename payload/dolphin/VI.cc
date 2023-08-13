extern "C" {
#include "VI.h"
}

#include <payload/Lock.hh>

extern "C" volatile u16 visel;

extern "C" u32 VIGetDTVStatus() {
    Lock<NoInterrupts> lock;
    return visel & 1;
}
