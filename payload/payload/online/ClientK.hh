#pragma once

#include <common/Array.hh>

class ClientK {
public:
    static void Init();
    static const Array<u8, 32> &Get();

private:
    ClientK();

    static Array<u8, 32> *s_instance;
};
