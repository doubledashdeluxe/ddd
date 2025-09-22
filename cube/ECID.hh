#pragma once

#include <portable/Array.hh>

class ECID {
public:
    static Array<u32, 4> Get();

private:
    ECID();
};
