#include "SRAM.hh"

#include <common/EXI.hh>
#include <portable/Bytes.hh>

SRAM::SRAM() : m_ok(false) {
    EXI::Device device(0, 1, 3);
    if (!device.ok()) {
        return;
    }
    u32 cmd = 0x20000100;
    if (!device.immWrite(&cmd, sizeof(cmd))) {
        return;
    }
    if (!device.immRead(m_buffer.values(), m_buffer.count())) {
        return;
    }
    m_ok = true;
}

SRAM::~SRAM() {}

bool SRAM::ok() const {
    return m_ok;
}

bool SRAM::getLanguage(u8 &language) const {
    if (!ok()) {
        return false;
    }

    language = Bytes::ReadBE<u8>(m_buffer.values(), 0x12);
    return true;
}
