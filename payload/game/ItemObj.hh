#pragma once

#include <jsystem/TVec3.hh>
#include <payload/Replace.hh>

class ItemObj {
public:
    class State {
    public:
        enum {
            Divested = 5,
            HeartWaiting = 9,
        };

    private:
        State();
    };

    u32 getKind() const;
    u32 getState() const;
    bool isSuccessionItem() const;
    s32 getSuccessionItemNum() const;
    void setStateDisappear(bool r4);

protected:
    const TVec3<f32> &REPLACED(getHandOffsetPos)();
    REPLACE const TVec3<f32> &getHandOffsetPos();

private:
    void updateHandOffsetPos();

    u8 _000[0x004 - 0x000];

public:
    TVec3<f32> m_pos;

private:
    u8 _010[0x118 - 0x010];
    u32 m_state;
    u8 _11c[0x120 - 0x11c];
    u32 m_kartIndex;
    u8 _124[0x128 - 0x124];
    Mtx34 *m_handAnmMtx;
    u8 _12c[0x260 - 0x12c];
    TVec3<f32> m_handOffsetPos;
    TVec3<f32> m_targetHandOffsetPos;
    u8 _278[0x2b0 - 0x278];
};
size_assert(ItemObj, 0x2b0);
