#include "TwoFields.hh"

#include <portable/Bytes.hh>

bool TwoFieldsReader::isValid(const u8 *buffer, u32 size, u32 &offset) {
    if (offset + 1 > size) {
        return false;
    }
    u32 firstCount0 = buffer[offset++];
    if (firstCount0 < 1 || firstCount0 > 3) {
        return false;
    }
    if (!isFirstCountValid(firstCount0)) {
        return false;
    }
    for (u32 i0 = 0; i0 < firstCount0; i0++) {
        if (offset + 4 > size) {
            return false;
        }
        if (!isFirstElementValid(i0, Bytes::ReadBE<u32>(buffer, offset))) {
            return false;
        }
        offset += 4;
    }
    if (!isSecondValid()) {
        return false;
    }
    return true;
}

void TwoFieldsReader::read(const u8 *buffer, u32 &offset) {
    u32 firstCount0 = buffer[offset++];
    setFirstCount(firstCount0);
    for (u32 i0 = 0; i0 < firstCount0; i0++) {
        setFirstElement(i0, Bytes::ReadBE<u32>(buffer, offset));
        offset += 4;
    }
    setSecond();
}

bool TwoFieldsWriter::write(u8 *buffer, u32 size, u32 &offset) {
    if (offset + 1 > size) {
        return false;
    }
    u32 firstCount0 = getFirstCount();
    if (firstCount0 < 1 || firstCount0 > 3) {
        return false;
    }
    buffer[offset++] = firstCount0;
    for (u32 i0 = 0; i0 < firstCount0; i0++) {
        if (offset + 4 > size) {
            return false;
        }
        Bytes::WriteBE<u32>(buffer, offset, getFirstElement(i0));
        offset += 4;
    }
    getSecond();
    return true;
}
