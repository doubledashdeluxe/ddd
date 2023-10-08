#pragma once

#include <common/Types.h>

s64 OSGetTime(void);
void OSSetTime(s64 time);
s64 OSGetSystemTime();

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
