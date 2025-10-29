#pragma once

#include <portable/crypto/Types.hh>

class ClientK {
public:
    static void Init();
    static const Key &Get();

private:
    ClientK();

    static Key *s_instance;
};
