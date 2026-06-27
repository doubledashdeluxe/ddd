#pragma once

#include "portable/Types.hh"

#ifdef __CWCC__
extern "C" {
#include <assert.h>
}
#endif

class Bytes {
public:
    template <typename T>
    static T ReadBE(const u8 *src, size_t offset) {
#ifdef __CWCC__
        T val;
        __memcpy(&val, src + offset, sizeof(val));
#else
        T val = 0;
        for (size_t i = 0; i < sizeof(T); i++) {
            val |= static_cast<T>(src[offset + i]) << (8 * (sizeof(T) - i - 1));
        }
#endif
        return val;
    }

    template <typename T>
    static T ReadLE(const u8 *src, size_t offset) {
#ifdef __CWCC__
        static_assert(false); // Poor codegen
#else
        T val = 0;
        for (size_t i = 0; i < sizeof(T); i++) {
            val |= static_cast<T>(src[offset + i]) << (8 * i);
        }
        return val;
#endif
    }

    template <typename T>
    static void WriteBE(u8 *dst, size_t offset, T val) {
#ifdef __CWCC__
        __memcpy(dst + offset, &val, sizeof(val));
#else
        for (size_t i = 0; i < sizeof(T); i++) {
            dst[offset + i] = val >> (8 * (sizeof(T) - i - 1));
        }
#endif
    }

    template <typename T>
    static void WriteLE(u8 *dst, size_t offset, T val) {
#ifdef __CWCC__
        static_assert(false); // Poor codegen
#else
        for (size_t i = 0; i < sizeof(T); i++) {
            dst[offset + i] = val >> (8 * i);
        }
#endif
    }

private:
    Bytes();
};

#ifdef __CWCC__
template <>
inline s64 Bytes::ReadBE(const u8 *src, size_t offset) {
    s64 val = 0;
    val |= static_cast<u64>(ReadBE<u32>(src, offset + 0)) << 32;
    val |= static_cast<u64>(ReadBE<u32>(src, offset + 4)) << 0;
    return val;
}

template <>
inline u64 Bytes::ReadBE(const u8 *src, size_t offset) {
    u64 val = 0;
    val |= static_cast<u64>(ReadBE<u32>(src, offset + 0)) << 32;
    val |= static_cast<u64>(ReadBE<u32>(src, offset + 4)) << 0;
    return val;
}

template <>
inline void Bytes::WriteBE(u8 *dst, size_t offset, s64 val) {
    WriteBE<s32>(dst, offset + 0, val >> 32);
    WriteBE<s32>(dst, offset + 4, val >> 0);
}

template <>
inline void Bytes::WriteBE(u8 *dst, size_t offset, u64 val) {
    WriteBE<u32>(dst, offset + 0, val >> 32);
    WriteBE<u32>(dst, offset + 4, val >> 0);
}

template <>
inline s8 Bytes::ReadLE(const u8 *src, size_t offset) {
    return src[offset];
}

template <>
inline s16 Bytes::ReadLE(const u8 *src, size_t offset) {
    return __lhbrx(src, offset);
}

template <>
inline s32 Bytes::ReadLE(const u8 *src, size_t offset) {
    return __lwbrx(src, offset);
}

template <>
inline s64 Bytes::ReadLE(const register u8 *src, register size_t offset) {
    register s64 val;
    register u32 x;

    asm {
        addi x, offset, 0x4;
        lwbrx val@loword, src, offset;
        lwbrx val@hiword, src, x;
    }

    return val;
}

template <>
inline u8 Bytes::ReadLE(const u8 *src, size_t offset) {
    return src[offset];
}

template <>
inline u16 Bytes::ReadLE(const u8 *src, size_t offset) {
    return __lhbrx(src, offset);
}

template <>
inline u32 Bytes::ReadLE(const u8 *src, size_t offset) {
    return __lwbrx(src, offset);
}

template <>
inline u64 Bytes::ReadLE(const register u8 *src, register size_t offset) {
    register u64 val;
    register u32 x;

    asm {
        addi x, offset, 0x4;
        lwbrx val@loword, src, offset;
        lwbrx val@hiword, src, x;
    }

    return val;
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, s8 val) {
    dst[offset] = val;
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, s16 val) {
    __sthbrx(val, dst, offset);
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, s32 val) {
    __stwbrx(val, dst, offset);
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, s64 val) {
    __stwbrx(val >> 32, dst, offset + 4);
    __stwbrx(val >> 0, dst, offset + 0);
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, u8 val) {
    dst[offset] = val;
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, u16 val) {
    __sthbrx(val, dst, offset);
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, u32 val) {
    __stwbrx(val, dst, offset);
}

template <>
inline void Bytes::WriteLE(u8 *dst, size_t offset, u64 val) {
    __stwbrx(val >> 32, dst, offset + 4);
    __stwbrx(val >> 0, dst, offset + 0);
}
#endif
