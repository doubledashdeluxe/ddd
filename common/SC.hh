#pragma once

#include <portable/Array.hh>
#include <portable/Bytes.hh>

class SC {
public:
    SC();
    SC(Array<u8, 0x4000> buffer);
    ~SC();
    bool ok() const;
    bool get(const char *name, u8 &value);
    bool get(const char *name, u16 &value);
    bool get(const char *name, u32 &value);
    bool get(const char *name, u64 &value);

private:
    template <typename T>
    bool get(u8 type, const char *name, T &value) {
        u16 offset;
        if (!getOffset(type, name, offset)) {
            return false;
        }
        if (offset + sizeof(T) > m_buffer.count()) {
            return false;
        }
        value = Bytes::ReadBE<T>(m_buffer.values(), offset);
        return true;
    }

    bool getOffset(u8 type, const char *name, u16 &offset);

    alignas(0x20) Array<u8, 0x4000> m_buffer;
};
