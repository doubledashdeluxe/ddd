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

class OneFieldReader {
public:
    bool isValid(const u8 *buffer, u32 size, u32 &offset);
    void read(const u8 *buffer, u32 &offset);

private:
    virtual bool isFirstValid(u8 first) = 0;
    virtual void setFirst(u8 first) = 0;
};

class OneFieldWriter {
public:
    bool write(u8 *buffer, u32 size, u32 &offset);

private:
    virtual u8 getFirst() = 0;
};
