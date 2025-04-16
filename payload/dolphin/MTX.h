#pragma once

#include <portable/Types.h>

typedef struct Vec3 {
    f32 x;
    f32 y;
    f32 z;
} Vec3;

typedef f32 Mtx34[3][4];

void C_MTXLookAt(Mtx34 m, const Vec3 *camPos, const Vec3 *camUp, const Vec3 *target);
