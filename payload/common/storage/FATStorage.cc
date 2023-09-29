#include <common/storage/FATStorage.hh>

extern "C" {
#include <dolphin/OSTime.h>

#include <assert.h>
}

Array<FATStorage *, FF_VOLUMES> FATStorage::s_volumes(nullptr); // TODO move to common

extern "C" DWORD get_fattime() {
    OSCalendarTime time;
    OSTicksToCalendarTime(OSGetTime(), &time);

    DWORD fTime = 0;

    assert(time.year >= 1980 && time.year < 2108);
    fTime |= (time.year - 1980) << 25;

    assert(time.mon >= 0 && time.mon < 12);
    fTime |= (time.mon + 1) << 21;

    assert(time.mday >= 1 && time.mday <= 31);
    fTime |= time.mday << 16;

    assert(time.hour >= 0 && time.hour < 24);
    fTime |= time.hour << 11;

    assert(time.min >= 0 && time.min < 60);
    fTime |= time.min << 5;

    assert(time.sec >= 0 && time.sec < 62);
    if (time.sec >= 60) {
        fTime |= 59 >> 1;
    } else {
        fTime |= time.sec >> 1;
    }

    return fTime;
}
