#pragma once

#include <common/Types.hh>

class OneFieldReader {
public:
    bool isValid(const u8 *buffer, u32 size, u32 &offset);
    void read(const u8 *buffer, u32 &offset);

private:
    virtual bool isFirstValid(u32 first) = 0;
    virtual void setFirst(u32 first) = 0;
};

class OneFieldWriter {
public:
    bool write(u8 *buffer, u32 size, u32 &offset);

private:
    virtual u32 getFirst() = 0;
};
