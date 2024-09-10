#pragma once

#include <common/Types.hh>

class TwoFieldsReader {
public:
    bool isValid(const u8 *buffer, u32 size, u32 &offset);
    void read(const u8 *buffer, u32 &offset);

private:
    virtual bool isFirstCountValid(u32 firstCount) = 0;
    virtual void setFirstCount(u32 firstCount) = 0;
    virtual bool isFirstElementValid(u32 i0, u32 firstElement) = 0;
    virtual void setFirstElement(u32 i0, u32 firstElement) = 0;
    virtual bool isSecondValid(u8 second) = 0;
    virtual void setSecond(u8 second) = 0;
};

class TwoFieldsWriter {
public:
    bool write(u8 *buffer, u32 size, u32 &offset);

private:
    virtual u32 getFirstCount() = 0;
    virtual u32 getFirstElement(u32 i0) = 0;
    virtual u8 getSecond() = 0;
};
