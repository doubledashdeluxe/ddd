#pragma once

#include <portable/Types.hh>

class MI {
public:
    enum {
        R = 1 << 0,
        W = 1 << 1,
    };

    static void Protect(u32 channel, u32 start, u32 end, u32 flags);
    static void Protect(u32 channel, void *start, void *end, u32 flags);

private:
    MI();
};
