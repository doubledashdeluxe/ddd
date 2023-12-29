#include <common/Clock.hh>

#include <common/Log.hh>
#include <common/SC.hh>
extern "C" {
#include <dolphin/EXIBios.h>
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
        WARN("Failed to read the counter bias!\n");
        counterBias = 189388800;
    }
    OSSetTime(SecondsToTicks(currRTC + counterBias));
}

void Clock::WaitMilliseconds(u32 milliseconds) {
    OSAlarm alarm;
    OSCreateAlarm(&alarm);
    alarm.userData = OSGetCurrentThread();
    Lock<NoInterrupts> lock;
    OSSetAlarm(&alarm, MillisecondsToTicks(milliseconds), HandleAlarm);
    OSSuspendThread(OSGetCurrentThread());
}

bool Clock::ReadRTC(u32 &rtc) {
    if (!EXILock(0, 1, nullptr)) {
        return false;
    }
    if (!EXISelect(0, 1, 3)) {
        EXIUnlock(0);
        return false;
    }
    u32 cmd = 0x20000000;
    if (!EXIImm(0, &cmd, sizeof(cmd), EXI_WRITE, nullptr)) {
        EXIDeselect(0);
        EXIUnlock(0);
        return false;
    }
    if (!EXISync(0)) {
        EXIDeselect(0);
        EXIUnlock(0);
        return false;
    }
    if (!EXIImm(0, &rtc, sizeof(rtc), EXI_READ, nullptr)) {
        EXIDeselect(0);
        EXIUnlock(0);
        return false;
    }
    if (!EXISync(0)) {
        EXIDeselect(0);
        EXIUnlock(0);
        return false;
    }
    if (!EXIDeselect(0)) {
        EXIUnlock(0);
        return false;
    }
    if (!EXIUnlock(0)) {
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
