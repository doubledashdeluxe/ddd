#include <common/Types.h>

u64 __udivdi3(u64 a, u64 b);
u64 __umoddi3(u64 a, u64 b);
s64 __ashldi3(s64 a, s32 b);
u64 __lshrdi3(u64 a, s32 b);

u64 __div2u(u64 a, u64 b) {
    return __udivdi3(a, b);
}

u64 __mod2u(u64 a, u64 b) {
    return __umoddi3(a, b);
}

s64 __shl2i(s64 a, s32 b) {
    return __ashldi3(a, b);
}

u64 __shr2u(u64 a, s32 b) {
    return __lshrdi3(a, b);
}
