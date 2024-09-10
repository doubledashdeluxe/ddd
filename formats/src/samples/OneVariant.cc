#include "OneVariant.hh"

#include <common/Bytes.hh>

bool OneVariantReader::isValid(const u8 *buffer, u32 size, u32 &offset) {
    if (offset + 1 > size) {
        return false;
    }
    u8 discriminant = buffer[offset++];
    switch (discriminant) {
    case 0:
        {
            if (offset + 4 > size) {
                return false;
            }
            if (!isFirstValid(Bytes::ReadBE<u32>(buffer, offset))) {
                return false;
            }
            offset += 4;
            return true;
        }
    default:
        return false;
    }
}

void OneVariantReader::read(const u8 *buffer, u32 &offset) {
    u8 discriminant = buffer[offset++];
    switch (discriminant) {
    case 0:
        {
            setFirst(Bytes::ReadBE<u32>(buffer, offset));
            offset += 4;
        }
    }
}

bool OneVariantWriter::First::write(u8 *buffer, u32 size, u32 &offset) {
    if (offset + 1 > size) {
        return false;
    }
    buffer[offset++] = 0;
    if (offset + 4 > size) {
        return false;
    }
    Bytes::WriteBE<u32>(buffer, offset, getFirst());
    offset += 4;
    return true;
}
