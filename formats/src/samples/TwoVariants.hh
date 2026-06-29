#pragma once

#include <portable/Bytes.hh>

template <typename D>
class TwoVariantsReader {
public:
    bool isValid(const u8 *buffer, u32 size, u32 &offset) {
        MaybeUnused(buffer);
        MaybeUnused(size);
        MaybeUnused(offset);
        D *d = static_cast<D *>(this);
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
                AssertType<bool (D::*)(u32)>(&D::isFirstCountValid);
                if (!d->isFirstCountValid(firstCount0)) {
                    return false;
                }
                for (u32 i0 = 0; i0 < firstCount0; i0++) {
                    if (offset + 4 > size) {
                        return false;
                    }
                    AssertType<bool (D::*)(u32, u32)>(&D::isFirstElementValid);
                    if (!d->isFirstElementValid(i0, Bytes::ReadBE<u32>(buffer, offset))) {
                        return false;
                    }
                    offset += 4;
                }
                return true;
            }
        case 1:
            {
                AssertType<bool (D::*)()>(&D::isSecondValid);
                if (!d->isSecondValid()) {
                    return false;
                }
                return true;
            }
        default:
            return false;
        }
    }

    void read(const u8 *buffer, u32 &offset) {
        MaybeUnused(buffer);
        MaybeUnused(offset);
        D *d = static_cast<D *>(this);
        u8 discriminant = buffer[offset++];
        switch (discriminant) {
        case 0:
            {
                u32 firstCount0 = buffer[offset++];
                AssertType<void (D::*)(u32)>(&D::setFirstCount);
                d->setFirstCount(firstCount0);
                for (u32 i0 = 0; i0 < firstCount0; i0++) {
                    AssertType<void (D::*)(u32, u32)>(&D::setFirstElement);
                    d->setFirstElement(i0, Bytes::ReadBE<u32>(buffer, offset));
                    offset += 4;
                }
                break;
            }
        case 1:
            {
                AssertType<void (D::*)()>(&D::setSecond);
                d->setSecond();
                break;
            }
        }
    }

    friend D;

private:
    TwoVariantsReader() {}
};

template <>
class TwoVariantsReader<void> {
public:
    bool isValid(const u8 * /* buffer */, u32 /* size */, u32 & /* offset */) {
        return false;
    }

    void read(const u8 * /* buffer */, u32 & /* offset */) {}
};

template <typename D>
class TwoVariantsWriter {
public:
    class First;
    class Second;

    virtual bool write(u8 *buffer, u32 size, u32 &offset) = 0;

    friend D;

private:
    TwoVariantsWriter() {}
};

template <typename D>
class TwoVariantsWriter<D>::First : public TwoVariantsWriter {
public:
    bool write(u8 *buffer, u32 size, u32 &offset) override {
        MaybeUnused(buffer);
        MaybeUnused(size);
        MaybeUnused(offset);
        D *d = static_cast<D *>(this);
        if (offset + 1 > size) {
            return false;
        }
        buffer[offset++] = 0;
        if (offset + 1 > size) {
            return false;
        }
        AssertType<u32 (D::*)()>(&D::getFirstCount);
        u32 firstCount0 = d->getFirstCount();
        if (firstCount0 < 1 || firstCount0 > 3) {
            return false;
        }
        buffer[offset++] = firstCount0;
        for (u32 i0 = 0; i0 < firstCount0; i0++) {
            if (offset + 4 > size) {
                return false;
            }
            AssertType<u32 (D::*)(u32)>(&D::getFirstElement);
            Bytes::WriteBE<u32>(buffer, offset, d->getFirstElement(i0));
            offset += 4;
        }
        return true;
    }

    friend D;

private:
    First() {}
};

template <typename D>
class TwoVariantsWriter<D>::Second : public TwoVariantsWriter {
public:
    bool write(u8 *buffer, u32 size, u32 &offset) override {
        MaybeUnused(buffer);
        MaybeUnused(size);
        MaybeUnused(offset);
        D *d = static_cast<D *>(this);
        if (offset + 1 > size) {
            return false;
        }
        buffer[offset++] = 1;
        AssertType<void (D::*)()>(&D::getSecond);
        d->getSecond();
        return true;
    }

    friend D;

private:
    Second() {}
};
