#pragma once

template <typename T>
const T &Upcast(const T &a) {
    return a;
}

template <typename T>
T &Upcast(T &a) {
    return a;
}
