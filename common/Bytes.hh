#pragma once

#include <common/Types.hh>

class Bytes {
public:
    template <typename T>
    static T ReadBE(const u8 *src, size_t offset) {
        T val = 0;
        for (size_t i = 0; i < sizeof(T); i++) {
            val |= static_cast<T>(src[offset + i]) << (8 * (sizeof(T) - i - 1));
        }
        return val;
    }

    template <typename T>
    static T ReadLE(const u8 *src, size_t offset) {
        T val = 0;
        for (size_t i = 0; i < sizeof(T); i++) {
            val |= static_cast<T>(src[offset + i]) << (8 * i);
        }
        return val;
    }

    template <typename T>
    static void WriteBE(u8 *dst, size_t offset, T val) {
        for (size_t i = 0; i < sizeof(T); i++) {
            dst[offset + i] = val >> (8 * (sizeof(T) - i - 1));
        }
    }

    template <typename T>
    static void WriteLE(u8 *dst, size_t offset, T val) {
        for (size_t i = 0; i < sizeof(T); i++) {
            dst[offset + i] = val >> (8 * i);
        }
    }

private:
    Bytes();
};
