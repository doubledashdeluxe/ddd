#include <cube/Clock.hh>

#include <cube/EXI.hh>
extern "C" {
#include <dolphin/OSAlarm.h>
#include <dolphin/OSRtc.h>
#include <dolphin/OSThread.h>
#include <dolphin/OSTime.h>
}
#include <payload/Lock.hh>

struct Alarm : OSAlarm {
    OSThread *thread;
};

void Clock::Init() {
    u32 prevRTC, currRTC;
    do {
        if (!ReadRTC(prevRTC)) {
            continue;
        }
        if (!ReadRTC(currRTC)) {
            continue;
        }
    } while (currRTC != prevRTC);
    u32 counterBias = OSGetCounterBias();
    OSSetTime(SecondsToTicks(currRTC + counterBias));
}

s64 Clock::GetMonotonicTicks() {
    return OSGetSystemTime();
}

void Clock::WaitTicks(s64 ticks) {
    Alarm alarm;
    OSCreateAlarm(&alarm);
    alarm.thread = OSGetCurrentThread();
    Lock<NoInterrupts> lock;
    OSSetAlarm(&alarm, ticks, HandleAlarm);
    OSSuspendThread(OSGetCurrentThread());
}

bool Clock::ReadRTC(u32 &rtc) {
    EXI::Device device(0, 1, 3, nullptr);
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

void Clock::HandleAlarm(OSAlarm *alarm, OSContext * /* context */) {
    OSResumeThread(static_cast<Alarm *>(alarm)->thread);
}
