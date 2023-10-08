#include <common/Clock.hh>

extern "C" {
#include <dolphin/OSThread.h>
}
#include <payload/Lock.hh>

void Clock::WaitMilliseconds(u32 milliseconds) {
    OSAlarm alarm;
    OSCreateAlarm(&alarm);
    alarm.userData = OSGetCurrentThread();
    Lock<NoInterrupts> lock;
    OSSetAlarm(&alarm, MillisecondsToTicks(milliseconds), HandleAlarm);
    OSSuspendThread(OSGetCurrentThread());
}

void Clock::HandleAlarm(OSAlarm *alarm, OSContext * /* context */) {
    OSResumeThread(reinterpret_cast<OSThread *>(alarm->userData));
}
