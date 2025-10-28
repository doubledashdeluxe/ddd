#pragma once

#include <portable/Types.hh>

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

class OneVariantReader {
public:
    bool isValid(const u8 *buffer, u32 size, u32 &offset);
    void read(const u8 *buffer, u32 &offset);

private:
    virtual bool isFirstValid(u8 first) = 0;
    virtual void setFirst(u8 first) = 0;
};

class OneVariantWriter {
public:
    class First;

    virtual bool write(u8 *buffer, u32 size, u32 &offset) = 0;
};

class OneVariantWriter::First : public OneVariantWriter {
public:
    bool write(u8 *buffer, u32 size, u32 &offset) override;

private:
    virtual u8 getFirst() = 0;
};
