#include "TwoVariants.hh"

#include <portable/Bytes.hh>

bool TwoVariantsReader::isValid(const u8 *buffer, u32 size, u32 &offset) {
    if (offset + 1 > size) {
        return false;
    }
    u8 discriminant = buffer[offset++];
    switch (discriminant) {
    case 0:
        {
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
            return true;
        }
    case 1:
        {
            if (offset + 1 > size) {
                return false;
            }
            if (!isSecondValid(Bytes::ReadBE<u8>(buffer, offset))) {
                return false;
            }
            offset += 1;
            return true;
        }
    default:
        return false;
    }
}

void TwoVariantsReader::read(const u8 *buffer, u32 &offset) {
    u8 discriminant = buffer[offset++];
    switch (discriminant) {
    case 0:
        {
            u32 firstCount0 = buffer[offset++];
            setFirstCount(firstCount0);
            for (u32 i0 = 0; i0 < firstCount0; i0++) {
                setFirstElement(i0, Bytes::ReadBE<u32>(buffer, offset));
                offset += 4;
            }
            break;
        }
    case 1:
        {
            setSecond(Bytes::ReadBE<u8>(buffer, offset));
            offset += 1;
            break;
        }
    }
}

bool TwoVariantsWriter::First::write(u8 *buffer, u32 size, u32 &offset) {
    if (offset + 1 > size) {
        return false;
    }
    buffer[offset++] = 0;
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
    return true;
}

bool TwoVariantsWriter::Second::write(u8 *buffer, u32 size, u32 &offset) {
    if (offset + 1 > size) {
        return false;
    }
    buffer[offset++] = 1;
    if (offset + 1 > size) {
        return false;
    }
    Bytes::WriteBE<u8>(buffer, offset, getSecond());
    offset += 1;
    return true;
}
