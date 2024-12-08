#pragma once

extern "C" {
#include <dolphin/MTX.h>
}

template <typename T>
struct TVec3 {
    TVec3() {}

    TVec3(T x, T y, T z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    T x;
    T y;
    T z;
};

template <>
struct TVec3<f32> : Vec3 {
    TVec3() {}

    TVec3(f32 x, f32 y, f32 z) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

template <typename T>
inline TVec3<T> operator+(const TVec3<T> &v0, const TVec3<T> &v1) {
    return TVec3<T>(v0.x + v1.x, v0.y + v1.y, v0.z + v1.z);
}

template <typename T>
inline TVec3<T> operator-(const TVec3<T> &v0, const TVec3<T> &v1) {
    return TVec3<T>(v0.x - v1.x, v0.y - v1.y, v0.z - v1.z);
}

template <typename T>
TVec3<T> operator*(const T &s, const TVec3<T> &v0) {
    return TVec3<T>(s * v0.x, s * v0.y, s * v0.z);
}

template <typename T>
inline TVec3<T> &operator+=(TVec3<T> &v, const TVec3<T> &v0) {
    v = v + v0;
    return v;
}

template <typename T>
inline TVec3<T> &operator-=(TVec3<T> &v, const TVec3<T> &v0) {
    v = v - v0;
    return v;
}

struct Vec3f : TVec3<f32> {
    Vec3f() {}

    Vec3f(f32 x, f32 y, f32 z) : TVec3(x, y, z) {}
};
