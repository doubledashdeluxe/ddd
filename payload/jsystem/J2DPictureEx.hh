#pragma once

#include "jsystem/J2DPicture.hh"

class J2DPictureEx : public J2DPicture {
public:
    J2DPictureEx();
    ~J2DPictureEx() override;
    // ...
    REPLACE void drawFullSet(f32 x, f32 y, f32 w, f32 h, Mtx34 *mtx) override;
    void drawTexCoord(f32 x, f32 y, f32 w, f32 h, s16 s0, s16 t0, s16 s1, s16 t1, s16 s2, s16 t2,
            s16 s3, s16 t3, Mtx34 *mtx) override;
    // ...

private:
    u8 _168[0x1a8 - 0x168];
};
size_assert(J2DPictureEx, 0x1a8);
