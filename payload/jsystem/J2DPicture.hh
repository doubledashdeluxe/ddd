#pragma once

#include "jsystem/J2DPane.hh"
#include "jsystem/ResTIMG.hh"

extern "C" {
#include <dolphin/GX.h>
#include <dolphin/MTX.h>
}

class J2DPicture : public J2DPane {
public:
    J2DPicture();
    ~J2DPicture() override;
    u32 getTypeID() const override;
    // ...
    virtual void vf_094();
    virtual void vf_098();
    virtual void vf_09c();
    virtual void vf_0a0();
    virtual void vf_0a4();
    virtual void vf_0a8();
    virtual void vf_0ac();
    virtual void vf_0b0();
    virtual void vf_0b4();
    virtual void vf_0b8();
    virtual void vf_0bc();
    virtual void vf_0c0();
    virtual void vf_0c4();
    virtual void vf_0c8();
    virtual void vf_0cc();
    virtual void vf_0d0();
    virtual void vf_0d4();
    virtual void vf_0d8();
    virtual void vf_0dc();
    virtual void vf_0e0();
    virtual void vf_0e4();
    virtual void vf_0e8();
    virtual void vf_0ec();
    virtual void vf_0f0();
    virtual void vf_0f4();
    virtual void vf_0f8();
    virtual void vf_0fc();
    virtual void vf_100();
    virtual void vf_104();
    virtual void vf_108();
    virtual void vf_10c();
    virtual ResTIMG *changeTexture(const ResTIMG *texture, u8 index);
    virtual ResTIMG *changeTexture(const char *name, u8 index);
    virtual void vf_118();
    virtual void vf_11c();
    virtual void vf_120();
    virtual void vf_124();
    virtual void vf_128();
    virtual void vf_12c();
    virtual bool setBlackWhite(GXColor black, GXColor white);
    virtual void vf_134();
    virtual void vf_138();
    virtual void vf_13c();
    REPLACE virtual void drawFullSet(f32 x, f32 y, f32 w, f32 h, Mtx34 *mtx);
    virtual void drawTexCoord(f32 x, f32 y, f32 w, f32 h, s16 s0, s16 t0, s16 s1, s16 t1, s16 s2,
            s16 t2, s16 s3, s16 t3, Mtx34 *mtx);
    virtual void vf_148();

    bool getBlackWhite(GXColor *black, GXColor *white) const;

    static u32 GetTypeID();

private:
    u8 _100[0x112 - 0x100];
    TVec2<s16> m_texCoords[4];
    u8 _122[0x168 - 0x122];
};
size_assert(J2DPicture, 0x168);
