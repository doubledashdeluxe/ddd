#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class DriverData {
public:
    static DriverData *REPLACED(Create)();
    REPLACE static DriverData *Create();

private:
    DriverData();

    u8 _00[0x7c - 0x00];
};
size_assert(DriverData, 0x7c);
