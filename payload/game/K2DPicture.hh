#pragma once

#include <jsystem/J2DPicture.hh>
#include <payload/Replace.hh>

class K2DPicture : public J2DPicture {
public:
    void REPLACED(drawK2D)(f32 x, f32 y, f32 w, f32 h, bool r4);
    REPLACE void drawK2D(f32 x, f32 y, f32 w, f32 h, bool r4);
};
size_assert(K2DPicture, 0x168);
