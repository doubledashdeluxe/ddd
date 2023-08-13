#pragma once

#include <common/Types.hh>

class Clock {
public:
    static void WaitMilliseconds(u32 milliseconds);

private:
    Clock();
};
