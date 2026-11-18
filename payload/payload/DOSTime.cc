#include "DOSTime.hh"

extern "C" {
#include <dolphin/OSTime.h>
}
#include <portable/Algorithm.hh>

u32 DOSTime::Now() {
    OSCalendarTime time;
    OSTicksToCalendarTime(OSGetTime(), &time);

    u32 dosTime = 0;

    time.year = Clamp(time.year, 1980, 2107);
    dosTime |= (time.year - 1980) << 25;

    time.mon = Clamp(time.mon, 0, 11);
    dosTime |= (time.mon + 1) << 21;

    time.mday = Clamp(time.mday, 1, 31);
    dosTime |= time.mday << 16;

    time.hour = Clamp(time.hour, 0, 23);
    dosTime |= time.hour << 11;

    time.min = Clamp(time.min, 0, 59);
    dosTime |= time.min << 5;

    time.sec = Clamp(time.sec, 0, 59);
    dosTime |= time.sec >> 1;

    return dosTime;
}
