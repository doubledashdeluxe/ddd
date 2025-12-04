#pragma once

#include <portable/Types.hh>

template <typename T>
struct TVec2 {
    TVec2() {}

    TVec2(T x, T y) {
        set(x, y);
    }

    void set(T x, T y) {
        this->x = x;
        this->y = y;
    }

    T x;
    T y;
};

template <typename T>
inline TVec2<T> operator+(const TVec2<T> &v0, const TVec2<T> &v1) {
    return TVec2<T>(v0.x + v1.x, v0.y + v1.y);
}

template <typename T>
inline TVec2<T> operator-(const TVec2<T> &v0, const TVec2<T> &v1) {
    return TVec2<T>(v0.x - v1.x, v0.y - v1.y);
}

template <typename T>
TVec2<T> operator*(const T &s, const TVec2<T> &v0) {
    return TVec2<T>(s * v0.x, s * v0.y);
}

struct Vec2f : TVec2<f32> {};
