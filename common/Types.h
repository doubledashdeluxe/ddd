#pragma once

typedef int BOOL;

typedef signed char s8;
typedef signed short s16;
typedef signed long s32;
typedef signed long long s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;

typedef unsigned long size_t;
typedef unsigned long uintptr_t;

// clang-format off
#define offsetof(type, member) ((size_t)&(((type *)0)->member))
// clang-format on
#define alignas(alignment) __attribute__((aligned(alignment)))
#define static_assert(cond) __static_assert((cond), #cond)
