#include "SC.hh"

#include "common/ios/File.hh"

SC::SC() : m_buffer(0) {
    IOS::File file("/shared2/sys/SYSCONF", IOS::Mode::Read);
    if (file.read(m_buffer.values(), m_buffer.count()) < 0) {
        Bytes::WriteBE<u32>(m_buffer.values(), 0x0, 0);
    }
}

SC::~SC() {}

bool SC::ok() const {
    u32 magic = 0x53437630;
    return Bytes::ReadBE<u32>(m_buffer.values(), 0x0) == magic;
}
