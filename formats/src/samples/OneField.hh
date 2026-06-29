#pragma once

#include <portable/Bytes.hh>

class First {
public:
    enum {
        A = 0,
        B = 1,
        Count = 2,
    };

private:
    First();
};

template <typename D>
class OneFieldReader {
public:
    bool isValid(const u8 *buffer, u32 size, u32 &offset) {
        MaybeUnused(buffer);
        MaybeUnused(size);
        MaybeUnused(offset);
        D *d = static_cast<D *>(this);
        if (offset + 1 > size) {
            return false;
        }
        u8 first = buffer[offset++];
        switch (first) {
        case 0:
        case 1:
            AssertType<bool (D::*)(u8)>(&D::isFirstValid);
            if (!d->isFirstValid(first)) {
                return false;
            }
            break;
        default:
            return false;
        }
        return true;
    }

    void read(const u8 *buffer, u32 &offset) {
        MaybeUnused(buffer);
        MaybeUnused(offset);
        D *d = static_cast<D *>(this);
        u8 first = buffer[offset++];
        AssertType<void (D::*)(u8)>(&D::setFirst);
        d->setFirst(first);
    }

    friend D;

private:
    OneFieldReader() {}
};

template <>
class OneFieldReader<void> {
public:
    bool isValid(const u8 * /* buffer */, u32 /* size */, u32 & /* offset */) {
        return false;
    }

    void read(const u8 * /* buffer */, u32 & /* offset */) {}
};

template <typename D>
class OneFieldWriter {
public:
    bool write(u8 *buffer, u32 size, u32 &offset) {
        MaybeUnused(buffer);
        MaybeUnused(size);
        MaybeUnused(offset);
        D *d = static_cast<D *>(this);
        if (offset + 1 > size) {
            return false;
        }
        AssertType<u8 (D::*)()>(&D::getFirst);
        u8 first = d->getFirst();
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

    friend D;

private:
    OneFieldWriter() {}
};
