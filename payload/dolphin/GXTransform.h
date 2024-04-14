#pragma once

#include <common/Types.h>

void GXSetViewport(f32 xOrig, f32 yOrig, f32 wd, f32 ht, f32 nearZ, f32 farZ);

void GXSetScissor(u32 xOrig, u32 yOrig, u32 wd, u32 ht);
void GXGetScissor(u32 *xOrig, u32 *yOrig, u32 *wd, u32 *ht);
