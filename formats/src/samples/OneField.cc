#include "OneField.hh"

#include <portable/Bytes.hh>

bool OneFieldReader::isValid(const u8 *buffer, u32 size, u32 &offset) {
    if (offset + 1 > size) {
        return false;
    }
    u8 first = buffer[offset++];
    switch (first) {
    case 0:
    case 1:
        if (!isFirstValid(first)) {
            return false;
        }
        break;
    default:
        return false;
    }
    return true;
}

void OneFieldReader::read(const u8 *buffer, u32 &offset) {
    u8 first = buffer[offset++];
    setFirst(first);
}

bool OneFieldWriter::write(u8 *buffer, u32 size, u32 &offset) {
    if (offset + 1 > size) {
        return false;
    }
    u8 first = getFirst();
    switch (first) {
    case 0:
    case 1:
        buffer[offset++] = first;
        break;
    default:
        return false;
    }
    return true;
}
