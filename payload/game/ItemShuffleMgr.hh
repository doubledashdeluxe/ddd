#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class ItemShuffleMgr {
public:
    bool REPLACED(doShuffle)(u32 kartIndex, u32 characterIndex);
    REPLACE bool doShuffle(u32 kartIndex, u32 characterIndex);
    u32 REPLACED(slotItem)(u32 kartIndex, u32 characterIndex);
    REPLACE u32 slotItem(u32 kartIndex, u32 characterIndex);
    void start();

private:
    u8 _0[0x4 - 0x0];
    u16 m_frame;
    u8 m_flags;
    bool m_isSpecialSlot;
    u8 _8[0xc - 0x8];
};
size_assert(ItemShuffleMgr, 0xc);
