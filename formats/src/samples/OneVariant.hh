#pragma once

#include <portable/Types.hh>

class OneVariantReader {
public:
    bool isValid(const u8 *buffer, u32 size, u32 &offset);
    void read(const u8 *buffer, u32 &offset);

private:
    virtual bool isFirstValid(u32 first) = 0;
    virtual void setFirst(u32 first) = 0;
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
    virtual u32 getFirst() = 0;
};
