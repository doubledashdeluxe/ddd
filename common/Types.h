#pragma once

#include <stddef.h>
#include <stdint.h>

typedef int BOOL;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

enum {
    FALSE = 0,
    TRUE = 1,
};

// clang-format off
#define offsetof(type, member) ((size_t)&(((type *)0)->member))
// clang-format on
#define alignas(alignment) __attribute__((aligned(alignment)))
#define size_assert(type, size) \
    __static_assert((sizeof(type) == size), "sizeof(" #type ") == " #size ")")
