#include "DOSTime.hh"

extern "C" {
#include <dolphin/OSTime.h>

#include <assert.h>
}

u32 DOSTime::Now() {
    OSCalendarTime time;
    OSTicksToCalendarTime(OSGetTime(), &time);

    u32 dosTime = 0;

    assert(time.year >= 1980 && time.year < 2108);
    dosTime |= (time.year - 1980) << 25;

    assert(time.mon >= 0 && time.mon < 12);
    dosTime |= (time.mon + 1) << 21;

    assert(time.mday >= 1 && time.mday <= 31);
    dosTime |= time.mday << 16;

    assert(time.hour >= 0 && time.hour < 24);
    dosTime |= time.hour << 11;

    assert(time.min >= 0 && time.min < 60);
    dosTime |= time.min << 5;

    assert(time.sec >= 0 && time.sec < 62);
    if (time.sec >= 60) {
        dosTime |= 59 >> 1;
    } else {
        dosTime |= time.sec >> 1;
    }

    return dosTime;
}
