#include <common/EXI.hh>

#include <common/Algorithm.hh>

extern "C" {
#include <string.h>
}

struct Channel {
    u32 cpr;
    u32 mar;
    u32 length;
    u32 cr;
    u32 data;
};
size_assert(Channel, 0x14);

extern "C" volatile Channel exi[3];

EXI::Device::Device(u32 channel, u32 device, u32 frequency) : m_channel(channel), m_ok(true) {
    exi[channel].cpr = (1 << device) << 7 | frequency << 4;
}

EXI::Device::~Device() {
    exi[m_channel].cpr = 0;
}

bool EXI::Device::immRead(void *buffer, u32 size) {
    while (size > 0) {
        u32 chunk = ~0;
        u32 chunkSize = Min<u32>(size, sizeof(chunk));
        exi[m_channel].data = chunk;
        exi[m_channel].cr = (chunkSize - 1) << 4 | 0 << 2 | 1 << 0;
        while (exi[m_channel].cr & 1) {}
        chunk = exi[m_channel].data;
        memcpy(buffer, &chunk, chunkSize);
        buffer = reinterpret_cast<u8 *>(buffer) + chunkSize;
        size -= chunkSize;
    }
    return true;
}

bool EXI::Device::immWrite(const void *buffer, u32 size) {
    while (size > 0) {
        u32 chunk = 0;
        u32 chunkSize = Min<u32>(size, sizeof(chunk));
        memcpy(&chunk, buffer, chunkSize);
        exi[m_channel].data = chunk;
        exi[m_channel].cr = (chunkSize - 1) << 4 | 1 << 2 | 1 << 0;
        while (exi[m_channel].cr & 1) {}
        buffer = reinterpret_cast<const u8 *>(buffer) + chunkSize;
        size -= chunkSize;
    }
    return true;
}

bool EXI::GetID(u32 channel, u32 device, u32 &id) {
    Device exiDevice(channel, device, 0);
    if (!exiDevice.ok()) {
        return false;
    }
    exi[channel].cpr = exi[channel].cpr | 1 << 11;
    u16 cmd = 0x0;
    if (!exiDevice.immWrite(&cmd, sizeof(cmd))) {
        return false;
    }
    if (!exiDevice.immRead(&id, sizeof(id))) {
        return false;
    }
    return true;
}
