#include <common/EXI.hh>

extern "C" {
#include <dolphin/EXIBios.h>
}

EXI::Device::Device(u32 channel, u32 device, u32 frequency) : m_channel(channel), m_ok(false) {
    if (!EXILock(channel, device, nullptr)) {
        return;
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
