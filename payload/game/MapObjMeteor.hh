#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class MapObjMeteor {
public:
    void REPLACED(showItem)();
    REPLACE void showItem();

private:
    u8 _000[0x190 - 0x000];
};
size_assert(MapObjMeteor, 0x190);
