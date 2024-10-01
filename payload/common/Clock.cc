#include <common/Clock.hh>

#include <common/EXI.hh>
#include <common/Log.hh>
#include <common/SC.hh>
extern "C" {
#include <dolphin/OSAlarm.h>
#include <dolphin/OSThread.h>
#include <dolphin/OSTime.h>
}
#include <payload/Lock.hh>

void Clock::Init() {
    u32 prevRTC, currRTC;
    while (true) {
        if (!ReadRTC(prevRTC)) {
            continue;
        }
        if (!ReadRTC(currRTC)) {
            continue;
        }
        if (currRTC == prevRTC) {
            break;
        }
    }
    u32 counterBias;
    if (!ReadCounterBias(counterBias)) {
        WARN("Failed to read the counter bias!");
        counterBias = 189388800;
    }
    OSSetTime(SecondsToTicks(currRTC + counterBias));
}

s64 Clock::GetMonotonicTicks() {
    return OSGetSystemTime();
}

void Clock::WaitTicks(s64 ticks) {
    OSAlarm alarm;
    OSCreateAlarm(&alarm);
    alarm.userData = OSGetCurrentThread();
    Lock<NoInterrupts> lock;
    OSSetAlarm(&alarm, ticks, HandleAlarm);
    OSSuspendThread(OSGetCurrentThread());
}

bool Clock::ReadRTC(u32 &rtc) {
    EXI::Device device(0, 1, 3);
    if (!device.ok()) {
        return false;
    }
    u32 cmd = 0x20000000;
    if (!device.immWrite(&cmd, sizeof(cmd))) {
        return false;
    }
    if (!device.immRead(&rtc, sizeof(rtc))) {
        return false;
    }
    return true;
}

bool Clock::ReadCounterBias(u32 &counterBias) {
    for (u32 i = 0; i < 5; i++) {
        SC sc;
        if (sc.get("IPL.CB", counterBias)) {
            return true;
        }

        WaitMilliseconds(200);
    }

    return false;
}

void Clock::HandleAlarm(OSAlarm *alarm, OSContext * /* context */) {
    OSResumeThread(reinterpret_cast<OSThread *>(alarm->userData));
}
