#pragma once

#include <portable/Types.hh>

class ItemObj {
public:
    u32 getKind() const;
    s32 getSuccessionItemNum() const;

private:
    u8 _000[0x2b0 - 0x000];
};
size_assert(ItemObj, 0x2b0);
