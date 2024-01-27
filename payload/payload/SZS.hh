#pragma once

#include <common/Types.hh>

class SZS {
public:
    static bool CheckMagic(const u8 *compressed, u32 compressedSize);
    static bool GetUncompressedSize(const u8 *compressed, u32 compressedSize,
            u32 &uncompressedSize);
    static bool Uncompress(const u8 *compressed, u32 compressedSize, u8 *uncompressed,
            u32 uncompressedSize);

private:
    SZS();
};
