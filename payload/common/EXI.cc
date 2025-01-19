#include <common/EXI.hh>

extern "C" {
#include <dolphin/EXIBios.h>
#include <dolphin/OSThread.h>
}
#include <payload/Lock.hh>

EXI::Device::Device(u32 channel, u32 device, u32 frequency) : m_channel(channel), m_ok(false) {
    {
        Lock<NoInterrupts> lock;

        while (!EXILock(channel, device, HandleUnlock)) {
            OSSleepThread(&s_queues[channel]);
        }
    }

    if (!EXISelect(channel, device, frequency)) {
        EXIUnlock(channel);
        return;
    }

    m_ok = true;
}

EXI::Device::~Device() {
    if (m_ok) {
        EXIDeselect(m_channel);
        EXIUnlock(m_channel);
    }
}

bool EXI::Device::immRead(void *buffer, u32 size) {
    return EXIImmEx(m_channel, buffer, size, EXI_READ);
}

bool EXI::Device::immWrite(const void *buffer, u32 size) {
    return EXIImmEx(m_channel, const_cast<void *>(buffer), size, EXI_WRITE);
}

bool EXI::GetID(u32 channel, u32 device, u32 &id) {
    return EXIGetID(channel, device, &id);
}

void EXI::HandleUnlock(s32 channel, OSContext * /* context */) {
    OSWakeupThread(&s_queues[channel]);
}

Array<OSThreadQueue, 3> EXI::s_queues;
