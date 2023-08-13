#pragma once

#include <common/Types.hh>

class DI {
public:
    static bool ReadDiscID();
    static bool Read(void *dst, u32 size, u32 offset);
    static bool IsInserted();
    static void Reset();

private:
    DI();
};
