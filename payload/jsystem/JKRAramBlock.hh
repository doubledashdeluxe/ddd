#pragma once

#include <common/Types.hh>

class JKRAramBlock {
public:
    JKRAramBlock(u32 r4, u32 r5, u32 r6, u8 r7, bool r8);
    virtual ~JKRAramBlock();

    u32 getAddress() const;

private:
    u8 _04[0x14 - 0x04];
    u32 m_address;
    u8 _18[0x24 - 0x18];
};
size_assert(JKRAramBlock, 0x24);
