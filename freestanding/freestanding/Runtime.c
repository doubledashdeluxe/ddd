#include <portable/Types.h>

#ifdef __CWCC__
u64 __udivdi3(u64 a, u64 b);
s64 __divdi3(s64 a, s64 b);
u64 __umoddi3(u64 a, u64 b);
s64 __ashldi3(s64 a, s32 b);
u64 __lshrdi3(u64 a, s32 b);

u64 __div2u(u64 a, u64 b) {
    return __udivdi3(a, b);
}

s64 __div2i(s64 a, s64 b) {
    return __divdi3(a, b);
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

typedef void (*Xtor)(void *object, s32 type);

void __construct_array(void *objects, Xtor ctor, Xtor /* dtor */, size_t size, size_t count) {
    for (size_t i = 0; i < count; i++) {
        void *object = (u8 *)objects + i * size;
        ctor(object, 1);
    }
}

void __destroy_arr(void *objects, Xtor dtor, size_t size, size_t count) {
    for (size_t i = count - 1; i-- > 0;) {
        void *object = (u8 *)objects + i * size;
        dtor(object, -1);
    }
}

asm void __ptmf_scall() {
    // clang-format off

    nofralloc

    // Get the thisptr offset
    lwz r0, 0x0 (r12)

    // Get the vf offset
    lwz r11, 0x4 (r12)

    // Get the function address or vtable offset
    lwz r12, 0x8 (r12)

    // Apply the thisptr offset
    add r3, r3, r0

    // Check whether the function is virtual
    cmpwi r11, 0x0
    blt jump

    // Get the vtable
    lwzx r12, r3, r12

    // Get the vf
    lwzx r12, r12, r11

jump:
    // Jump to the function
    mtctr r12
    bctr

    // clang-format on
}
#endif
