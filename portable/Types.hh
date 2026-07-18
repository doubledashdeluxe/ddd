#pragma once

extern "C" {
#include "portable/Types.h"
}

template <size_t Alignment>
struct Aligner {
    u8 aligner[0 - Alignment];
};

template <>
struct Aligner<1> {
    u8 aligner;
};
alignment_assert(Aligner<1>, 1);

template <>
struct Aligner<2> {
    u16 aligner;
};
alignment_assert(Aligner<2>, 2);

template <>
struct Aligner<4> {
    u32 aligner;
};
alignment_assert(Aligner<4>, 4);

template <>
struct Aligner<8> {
    f64 aligner;
};
alignment_assert(Aligner<8>, 8);

template <typename T>
void AssertType(const T &) {}

template <typename T, typename U>
void AssertType2(const T &) {}

template <typename T, typename U>
void AssertType2(const U &) {}

template <typename T>
void MaybeUnused(const T &) {}
