#pragma once

#include <portable/Types.h>

u32 OSGetTick(void);
s64 OSGetTime(void);
void OSSetTime(s64 time);
s64 OSGetSystemTime(void);

typedef struct {
    int sec;
    int min;
    int hour;
    int mday;
    int mon;
    int year;
    int wday;
    int yday;

    int msec;
    int usec;
} OSCalendarTime;

void OSTicksToCalendarTime(s64 ticks, OSCalendarTime *td);
