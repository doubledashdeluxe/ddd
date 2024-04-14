#pragma once

#include "jsystem/J2DGraphContext.hh"
#include "jsystem/J2DMaterial.hh"
#include "jsystem/J2DPane.hh"
#include "jsystem/JKRArchive.hh"

class J2DScreen : public J2DPane {
public:
    J2DScreen();
    ~J2DScreen() override;
    u32 getTypeID() const override;
    void calcMtx() override;
    void vf_38() override;
    J2DPane *search(u64 tag) override;
    void vf_40() override;
    void vf_4c() override;
    void vf_50() override;
    void vf_54() override;
    void setAnimation(J2DAnmBase *animation) override;
    // ...

    J2DPane *search(const char *format, ...);

    void set(const char *file, u32 flags, JKRArchive *archive);
    void setPriority(const char *file, u32 flags, JKRArchive *archive);
    void draw(f32 f1, f32 f2, const J2DGraphContext *graphContext);
    void animation();
    void animationMaterials();

    static u32 GetTypeID();

private:
    u8 _100[0x102 - 0x100];
    u16 m_materialCount;
    J2DMaterial *m_materials;
    u8 _108[0x118 - 0x108];
};
size_assert(J2DScreen, 0x118);
