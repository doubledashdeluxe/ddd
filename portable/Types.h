#pragma once

#include <stdbool.h>
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
#ifndef FALSE
    FALSE = 0,
#endif
#ifndef TRUE
    TRUE = 1,
#endif
};

#define container_of(ptr, type, member) \
    ((type *)((char *)(1 ? (ptr) : &((type *)0)->member) - offsetof(type, member)))
#ifdef __CWCC__
#define alignas(alignment) __attribute__((aligned(alignment)))
#define size_assert(type, size) \
    __static_assert((sizeof(type) == size), "sizeof(" #type ") == " #size)
#define alignment_assert(type, alignment) \
    __static_assert(alignof(type) == alignment, "alignof(" #type ") == " #alignment)
#else
#define size_assert(type, size)
#define alignment_assert(type, alignment) static_assert(alignof(type) == alignment)
#endif
