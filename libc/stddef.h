#pragma once

#ifdef __cplusplus
#define NULL nullptr
#else
#define NULL ((void *)0)
#endif

#ifdef __CWCC__
// clang-format off
#define offsetof(type, member) ((size_t)&(((type *)0)->member))
// clang-format on
#else
#define offsetof __builtin_offsetof
#endif

typedef unsigned long size_t;
