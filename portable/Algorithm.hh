#pragma once

#include "portable/Types.hh"

template <typename T>
const T &Min(const T &a, const T &b) {
    return (b < a) ? b : a;
}

template <typename T>
const T &Max(const T &a, const T &b) {
    return (a < b) ? b : a;
}

template <typename T>
void Swap(T &a, T &b) {
    T tmp = a;
    a = b;
    b = tmp;
}

template <typename T>
struct Less {
    bool operator()(const T &a, const T &b) {
        return a < b;
    }
};

template <typename T>
struct Greater {
    bool operator()(const T &a, const T &b) {
        return a > b;
    }
};

template <typename S>
void Sort(S &sequence, size_t count) {
    Sort(sequence, count, Less<decltype(sequence[0])>());
}

template <typename S, typename C>
void Sort(S &sequence, size_t count, C compare) {
    for (size_t partSize = 2; partSize / 2 < count; partSize *= 2) {
        for (size_t partStart = 0; partStart + partSize / 2 < count; partStart += partSize) {
            for (size_t gap = partSize / 2; gap > 0; gap /= 2) {
                for (size_t index = partStart; index + gap < count; index++) {
                    if (compare(sequence[index + gap], sequence[index])) {
                        Swap(sequence[index], sequence[index + gap]);
                    }
                }
            }
        }
    }
}

template <typename S>
void Reverse(S &sequence, size_t count, size_t offset = 0) {
    for (size_t i = 0; i < count / 2; i++) {
        Swap(sequence[offset + i], sequence[offset + count - 1 - i]);
    }
}

template <typename S>
void RotateLeft(S &sequence, size_t count, size_t amount) {
    if (count != 0 && amount != 0) {
        amount %= count;
        Reverse(sequence, amount, 0);
        Reverse(sequence, count - amount, amount);
        Reverse(sequence, count);
    }
}

template <typename S>
void RotateRight(S &sequence, size_t count, size_t amount) {
    if (count != 0 && amount != 0) {
        amount %= count;
        Reverse(sequence, count - amount, 0);
        Reverse(sequence, amount, count - amount);
        Reverse(sequence, count);
    }
}
