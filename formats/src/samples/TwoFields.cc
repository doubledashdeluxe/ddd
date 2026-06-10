#include "TwoFields.hh"

#include <portable/Bytes.hh>

bool TwoFieldsReader::isValid(const u8 *buffer, u32 size, u32 &offset) {
    for (u32 i0 = 0; i0 < 3; i0++) {
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
    for (u32 i0 = 0; i0 < 3; i0++) {
        setFirstElement(i0, Bytes::ReadBE<u32>(buffer, offset));
        offset += 4;
    }
    setSecond();
}

bool TwoFieldsWriter::write(u8 *buffer, u32 size, u32 &offset) {
    for (u32 i0 = 0; i0 < 3; i0++) {
        if (offset + 4 > size) {
            return false;
        }
        Bytes::WriteBE<u32>(buffer, offset, getFirstElement(i0));
        offset += 4;
    }
    getSecond();
    return true;
}
