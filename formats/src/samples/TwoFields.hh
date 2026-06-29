#pragma once

#include <portable/Bytes.hh>

template <typename D>
class TwoFieldsReader {
public:
    bool isValid(const u8 *buffer, u32 size, u32 &offset) {
        MaybeUnused(buffer);
        MaybeUnused(size);
        MaybeUnused(offset);
        D *d = static_cast<D *>(this);
        for (u32 i0 = 0; i0 < 3; i0++) {
            if (offset + 4 > size) {
                return false;
            }
            AssertType<bool (D::*)(u32, u32)>(&D::isFirstElementValid);
            if (!d->isFirstElementValid(i0, Bytes::ReadBE<u32>(buffer, offset))) {
                return false;
            }
            offset += 4;
        }
        AssertType<bool (D::*)()>(&D::isSecondValid);
        if (!d->isSecondValid()) {
            return false;
        }
        return true;
    }

    void read(const u8 *buffer, u32 &offset) {
        MaybeUnused(buffer);
        MaybeUnused(offset);
        D *d = static_cast<D *>(this);
        for (u32 i0 = 0; i0 < 3; i0++) {
            AssertType<void (D::*)(u32, u32)>(&D::setFirstElement);
            d->setFirstElement(i0, Bytes::ReadBE<u32>(buffer, offset));
            offset += 4;
        }
        AssertType<void (D::*)()>(&D::setSecond);
        d->setSecond();
    }

    friend D;

private:
    TwoFieldsReader() {}
};

template <>
class TwoFieldsReader<void> {
public:
    bool isValid(const u8 * /* buffer */, u32 /* size */, u32 & /* offset */) {
        return false;
    }

    void read(const u8 * /* buffer */, u32 & /* offset */) {}
};

template <typename D>
class TwoFieldsWriter {
public:
    bool write(u8 *buffer, u32 size, u32 &offset) {
        MaybeUnused(buffer);
        MaybeUnused(size);
        MaybeUnused(offset);
        D *d = static_cast<D *>(this);
        for (u32 i0 = 0; i0 < 3; i0++) {
            if (offset + 4 > size) {
                return false;
            }
            AssertType<u32 (D::*)(u32)>(&D::getFirstElement);
            Bytes::WriteBE<u32>(buffer, offset, d->getFirstElement(i0));
            offset += 4;
        }
        AssertType<void (D::*)()>(&D::getSecond);
        d->getSecond();
        return true;
    }

    friend D;

private:
    TwoFieldsWriter() {}
};
