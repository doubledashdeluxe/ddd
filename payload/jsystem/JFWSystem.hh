#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

class JFWSystem {
public:
    static void REPLACED(Init)();
    REPLACE static void Init();

private:
    JFWSystem();

    static size_t s_systemHeapSize;
};
size_assert(JFWSystem, 0x1);
