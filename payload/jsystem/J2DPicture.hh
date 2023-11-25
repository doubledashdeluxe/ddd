#pragma once

#include "jsystem/J2DPane.hh"
#include "jsystem/ResTIMG.hh"

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

    static u32 GetTypeID();

private:
    u8 _100[0x168 - 0x100];
};
size_assert(J2DPicture, 0x168);
