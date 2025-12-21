#pragma once

#include <portable/Types.hh>

class MotorManager {
public:
    void exec();

    static MotorManager *Instance();

private:
    u8 _00[0x20 - 0x00];

    static MotorManager *s_instance;
};
size_assert(MotorManager, 0x20);
