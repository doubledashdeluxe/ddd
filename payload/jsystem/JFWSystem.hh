#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class JFWSystem {
public:
    static void REPLACED(Init)();
    REPLACE static void Init();

private:
    JFWSystem();

    static size_t s_systemHeapSize;
};
size_assert(JFWSystem, 0x1);
