#include "SC.hh"

#include "common/ios/File.hh"

extern "C" {
#include <string.h>
}

SC::SC() : m_buffer(0) {
    IOS::File file("/shared2/sys/SYSCONF", IOS::Mode::Read);
    if (!file.ok() || file.read(m_buffer.values(), m_buffer.count()) < 0) {
        Bytes::WriteBE<u32>(m_buffer.values(), 0x0, 0);
    }
}

SC::~SC() {}

bool SC::ok() const {
    u32 magic = 0x53437630;
    return Bytes::ReadBE<u32>(m_buffer.values(), 0x0) == magic;
}

bool SC::get(const char *name, u8 &value) {
    return get(3, name, value);
}

bool SC::get(const char *name, u16 &value) {
    return get(4, name, value);
}

bool SC::get(const char *name, u32 &value) {
    return get(5, name, value);
}

bool SC::get(const char *name, u64 &value) {
    return get(6, name, value);
}

bool SC::getOffset(u8 type, const char *name, u16 &offset) {
    if (!ok()) {
        return false;
    }

    u16 count = Bytes::ReadBE<u16>(m_buffer.values(), 0x4);
    for (u16 i = 0; i < count; i++) {
        offset = 0x6 + i * 0x2;
        if (offset + sizeof(u16) + sizeof(u8) > m_buffer.count()) {
            return false;
        }
        offset = Bytes::ReadBE<u16>(m_buffer.values(), offset);
        if (offset + sizeof(u8) > m_buffer.count()) {
            return false;
        }
        u8 lengthAndType = Bytes::ReadBE<u8>(m_buffer.values(), offset);
        if (lengthAndType >> 5 != type) {
            continue;
        }
        u8 length = (lengthAndType & 0x1f) + 0x1;
        if (static_cast<size_t>(offset + 0x1 + length) > m_buffer.count()) {
            return false;
        }
        if (memcmp(m_buffer.values() + offset + 0x1, name, length)) {
            continue;
        }
        offset += 0x1 + length;
        return true;
    }

    return false;
}
