#pragma once

template <typename T>
const T &Min(const T &a, const T &b) {
    return (b < a) ? b : a;
}

template <typename T>
const T &Max(const T &a, const T &b) {
    return (a < b) ? b : a;
}
