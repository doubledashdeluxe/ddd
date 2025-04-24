#include "SCFile.hh"

#include "common/ios/File.hh"

SCFile::SCFile() : SC(m_buffer), m_buffer(0) {
    IOS::File file("/shared2/sys/SYSCONF", IOS::Mode::Read);
    if (!file.ok() || file.read(m_buffer.values(), m_buffer.count()) < 0) {
        Bytes::WriteBE<u32>(m_buffer.values(), 0x0, 0);
    }
}

SCFile::~SCFile() {}
