#include "OneField.hh"

#include <portable/Bytes.hh>

bool OneFieldReader::isValid(const u8 *buffer, u32 size, u32 &offset) {
    if (offset + 4 > size) {
        return false;
    }
    if (!isFirstValid(Bytes::ReadBE<u32>(buffer, offset))) {
        return false;
    }
    offset += 4;
    return true;
}

void OneFieldReader::read(const u8 *buffer, u32 &offset) {
    setFirst(Bytes::ReadBE<u32>(buffer, offset));
    offset += 4;
}

bool OneFieldWriter::write(u8 *buffer, u32 size, u32 &offset) {
    if (offset + 4 > size) {
        return false;
    }
    Bytes::WriteBE<u32>(buffer, offset, getFirst());
    offset += 4;
    return true;
}
