#include "TwoFields.hh"

#include <portable/Bytes.hh>

bool TwoFieldsReader::isValid(const u8 *buffer, u32 size, u32 &offset) {
    if (offset + 1 > size) {
        return false;
    }
    u32 c0 = buffer[offset++];
    if (c0 < 1 || c0 > 3) {
        return false;
    }
    if (!isFirstCountValid(c0)) {
        return false;
    }
    for (u32 i0 = 0; i0 < c0; i0++) {
        if (offset + 4 > size) {
            return false;
        }
        if (!isFirstElementValid(i0, Bytes::ReadBE<u32>(buffer, offset))) {
            return false;
        }
        offset += 4;
    }
    if (offset + 1 > size) {
        return false;
    }
    if (!isSecondValid(Bytes::ReadBE<u8>(buffer, offset))) {
        return false;
    }
    offset += 1;
    return true;
}

void TwoFieldsReader::read(const u8 *buffer, u32 &offset) {
    u32 c0 = buffer[offset++];
    setFirstCount(c0);
    for (u32 i0 = 0; i0 < c0; i0++) {
        setFirstElement(i0, Bytes::ReadBE<u32>(buffer, offset));
        offset += 4;
    }
    setSecond(Bytes::ReadBE<u8>(buffer, offset));
    offset += 1;
}

bool TwoFieldsWriter::write(u8 *buffer, u32 size, u32 &offset) {
    if (offset + 1 > size) {
        return false;
    }
    u32 c0 = getFirstCount();
    if (c0 < 1 || c0 > 3) {
        return false;
    }
    buffer[offset++] = c0;
    for (u32 i0 = 0; i0 < c0; i0++) {
        if (offset + 4 > size) {
            return false;
        }
        Bytes::WriteBE<u32>(buffer, offset, getFirstElement(i0));
        offset += 4;
    }
    if (offset + 1 > size) {
        return false;
    }
    Bytes::WriteBE<u8>(buffer, offset, getSecond());
    offset += 1;
    return true;
}
