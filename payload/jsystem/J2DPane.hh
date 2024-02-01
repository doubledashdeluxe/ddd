#pragma once

#include "jsystem/J2DAnmBase.hh"
#include "jsystem/TVec2.hh"

class J2DPane {
public:
    class TypeID {
    public:
        enum {
            Screen = 0x8,
            Pane = 0x10,
            Window = 0x11,
            Picture = 0x12,
            TextBox = 0x13,
        };

    private:
        TypeID();
    };

    J2DPane();
    virtual ~J2DPane();
    virtual u32 getTypeID() const;
    virtual void vf_10();
    virtual void vf_14();
    virtual void vf_18();
    virtual void vf_1c();
    virtual void vf_20();
    virtual void setAlpha(u8 alpha);
    virtual void vf_28();
    virtual void calcMtx();
    virtual void vf_30();
    virtual void vf_34();
    virtual void vf_38();
    virtual J2DPane *search(u64 tag);
    virtual void vf_40();
    virtual void vf_44();
    virtual void vf_48();
    virtual void vf_4c();
    virtual void vf_50();
    virtual void vf_54();
    virtual void vf_58();
    virtual void setAnimation(J2DAnmBase *animation);
    virtual void vf_60();
    virtual void vf_64();
    virtual void vf_68();
    virtual void vf_6c();
    virtual void vf_70();
    virtual void vf_74();
    virtual void vf_78();
    virtual void vf_7c();
    virtual void vf_80();
    virtual void vf_84();
    virtual void vf_88();
    virtual void vf_8c();
    virtual void vf_90();

    template <typename P>
    const P *downcast() const {
        return reinterpret_cast<const P *>(getTypeID() == P::GetTypeID() ? this : nullptr);
    }

    template <typename P>
    P *downcast() {
        return reinterpret_cast<P *>(getTypeID() == P::GetTypeID() ? this : nullptr);
    }

    J2DPane *search(const char *tag);
    bool appendChild(J2DPane *child);

    static u32 GetTypeID();

private:
    u8 _004[0x0b0 - 0x004];

public:
    bool m_isVisible;

private:
    u8 _0b1[0x0d4 - 0x0b1];

public:
    TVec2<f32> m_offset;

private:
    u8 _0dc[0x100 - 0x0dc];
};
size_assert(J2DPane, 0x100);
