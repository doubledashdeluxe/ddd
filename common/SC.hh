#pragma once

#include "common/Array.hh"
#include "common/Bytes.hh"

extern "C" {
#include <string.h>
}

class SC {
public:
    template <typename T>
    struct TypeHelper;

    SC();
    ~SC();
    bool ok() const;

    template <typename T>
    bool get(const char *name, T &value) {
        if (!ok()) {
            return false;
        }

        u16 count = Bytes::ReadBE<u16>(m_buffer.values(), 0x4);
        for (u16 i = 0; i < count; i++) {
            u16 offset = 0x6 + i * 0x2;
            if (offset + sizeof(u16) + sizeof(u8) > m_buffer.count()) {
                return false;
            }
            offset = Bytes::ReadBE<u16>(m_buffer.values(), offset);
            if (offset + sizeof(u8) > m_buffer.count()) {
                return false;
            }
            u8 lengthAndType = Bytes::ReadBE<u8>(m_buffer.values(), offset);
            if (lengthAndType >> 5 != TypeHelper<T>::ID) {
                continue;
            }
            u8 length = (lengthAndType & 0x1f) + 0x1;
            if (static_cast<size_t>(offset + 0x1 + length) > m_buffer.count()) {
                return false;
            }
            if (memcmp(m_buffer.values() + offset + 0x1, name, length)) {
                continue;
            }
            if (offset + 0x1 + length + sizeof(T) > m_buffer.count()) {
                return false;
            }
            value = Bytes::ReadBE<T>(m_buffer.values(), offset + 0x1 + length);
            return true;
        }

        return false;
    }

private:
    alignas(0x20) Array<u8, 0x4000> m_buffer;
};

template <>
struct SC::TypeHelper<u8> {
    static const u8 ID = 3;
};

template <>
struct SC::TypeHelper<u16> {
    static const u8 ID = 4;
};

template <>
struct SC::TypeHelper<u32> {
    static const u8 ID = 5;
};

template <>
struct SC::TypeHelper<u64> {
    static const u8 ID = 6;
};
