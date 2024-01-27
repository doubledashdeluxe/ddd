#include "SZS.hh"

#include <common/Bytes.hh>

extern "C" {
#include <string.h>
}

bool SZS::CheckMagic(const u8 *compressed, u32 compressedSize) {
    if (compressedSize < 0x10) {
        return false;
    }

    u32 magic;
    memcpy(&magic, compressed, sizeof(magic));
    if (memcmp(&magic, "Yaz0", sizeof(magic)) && memcmp(&magic, "Yaz1", sizeof(magic))) {
        return false;
    }

    return true;
}

bool SZS::GetUncompressedSize(const u8 *compressed, u32 compressedSize, u32 &uncompressedSize) {
    if (!CheckMagic(compressed, compressedSize)) {
        return false;
    }

    uncompressedSize = Bytes::ReadBE<u32>(compressed, 0x04);
    return true;
}

bool SZS::Uncompress(const u8 *compressed, u32 compressedSize, u8 *uncompressed,
        u32 uncompressedSize) {
    if (!CheckMagic(compressed, compressedSize)) {
        return false;
    }

    u32 compressedOffset = 0x10, uncompressedOffset = 0x0;
    u8 groupHeader;
    for (u8 i = 0; compressedOffset < compressedSize && uncompressedOffset < uncompressedSize;
            i = (i + 1) % 8) {
        if (i == 0) {
            groupHeader = Bytes::ReadBE<u8>(compressed, compressedOffset);
            compressedOffset += sizeof(u8);
            if (compressedOffset == compressedSize) {
                break;
            }
        }
        if (groupHeader >> (7 - i) & 1) {
            uncompressed[uncompressedOffset++] = compressed[compressedOffset++];
        } else {
            if (compressedOffset + 2 > compressedSize) {
                break;
            }
            u16 val = Bytes::ReadBE<u16>(compressed, compressedOffset);
            compressedOffset += sizeof(u16);
            u32 referenceOffset = uncompressedOffset - (val & 0xfff) - 0x1;
            if (referenceOffset >= uncompressedOffset) {
                return false;
            }
            u16 referenceSize = (val >> 12) + 0x2;
            if (referenceSize == 0x2) {
                if (compressedOffset + 1 > compressedSize) {
                    break;
                }
                referenceSize = Bytes::ReadBE<u8>(compressed, compressedOffset) + 0x12;
                compressedOffset += sizeof(u8);
            }
            if (uncompressedOffset + referenceSize > uncompressedSize) {
                referenceSize = uncompressedSize - uncompressedOffset;
            }
            for (u16 j = 0; j < referenceSize; j++) {
                uncompressed[uncompressedOffset + j] = uncompressed[referenceOffset + j];
            }
            uncompressedOffset += referenceSize;
        }
    }

    return uncompressedOffset == uncompressedSize;
}
