#pragma once

#include <portable/Types.h>

typedef struct Vec3 {
    f32 x;
    f32 y;
    f32 z;
} Vec3;

typedef f32 Mtx34[3][4];

void PSMTXConcat(const Mtx34 a, const Mtx34 b, Mtx34 ab);
void PSMTXRotRad(Mtx34 m, char axis, f32 rad);
void C_MTXLookAt(Mtx34 m, const Vec3 *camPos, const Vec3 *camUp, const Vec3 *target);
