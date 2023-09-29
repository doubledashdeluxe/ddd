#pragma once

#include "common/Types.hh"

#ifdef PAYLOAD
extern "C" {
#include <dolphin/OSAlarm.h>
}
#endif

class Clock {
public:
    static void WaitMilliseconds(u32 milliseconds);

private:
    Clock();

#ifdef PAYLOAD
    static void HandleAlarm(OSAlarm *alarm, OSContext *context);
#endif
};
