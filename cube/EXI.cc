#include "EXI.hh"

EXI::Device::Device() : m_ok(false) {}

EXI::Device::Device(u32 channel, u32 device, u32 frequency, bool *wasDetached) {
    acquire(channel, device, frequency, wasDetached);
}

EXI::Device::~Device() {
    release();
}

bool EXI::Device::ok() const {
    return m_ok;
}

bool EXI::CanSwap(u32 channel, u32 device) {
    return channel != 2 && device == 0;
}
