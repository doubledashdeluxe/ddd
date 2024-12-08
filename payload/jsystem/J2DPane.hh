#pragma once

#include "jsystem/J2DAnmBase.hh"
#include "jsystem/TBox.hh"
#include "jsystem/TVec2.hh"

extern "C" {
#include <dolphin/MTX.h>
}
#include <payload/Replace.hh>

extern "C" {
#include <stdarg.h>
}

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

    const TBox<f32> &getBox() const;
    const TBox<f32> &getGlobalBox() const;
    J2DPane *getFirstChildPane();
    J2DPane *getNextChildPane();
    J2DPane *search(const char *format, ...);
    J2DPane *vsearch(const char *format, va_list vlist);
    bool appendChild(J2DPane *child);
    void setHasARScissor(bool hasARScissor, bool recursive);
    void setHasARTrans(bool hasARTrans, bool recursive);
    void setHasARShift(bool hasARShift, bool recursive);
    void setHasARShiftLeft(bool hasARShiftLeft, bool recursive);
    void setHasARShiftRight(bool hasARShiftRight, bool recursive);
    void setHasARScale(bool hasARScale, bool recursive);
    void setHasARTexCoords(bool hasARTexCoords, bool recursive);

    static u32 GetTypeID();
    static s32 GetARShift();
    static f32 GetARScale();
    static void SetAspectRatio(f32 aspectRatio);

private:
    typedef void (J2DPane::*BoolSetter)(bool value, bool recursive);

    void REPLACED(changeUseTrans)(J2DPane *parent);
    REPLACE void changeUseTrans(J2DPane *parent);

    void set(BoolSetter setter, bool value, bool recursive);

    u8 _004[0x006 - 0x004];

protected:
    bool m_hasARScissor : 1;
    bool m_hasARTrans : 1;
    bool m_hasARShift : 1;
    bool m_hasARShiftLeft : 1;
    bool m_hasARShiftRight : 1;
    bool m_hasARScale : 1;
    bool m_hasARTexCoords : 1;

private:
    u8 _007[0x020 - 0x007];

protected:
    TBox<f32> m_box;
    TBox<f32> m_globalBox;

private:
    u8 _040[0x080 - 0x040];

protected:
    Mtx34 m_globalMtx;

public:
    bool m_isVisible;

private:
    u8 _0b1[0x0d4 - 0x0b1];

public:
    TVec2<f32> m_offset;

private:
    u8 _0dc[0x100 - 0x0dc];

protected:
    static f32 s_aspectRatio;
    static u32 s_arWidth;
    static s32 s_arShift;
    static f32 s_arScale;
};
size_assert(J2DPane, 0x100);
