#pragma once

#include "game/KartBody.hh"

#include <jsystem/TVec3.hh>
#include <payload/Replace.hh>

class KartCam {
public:
    KartBody *getBody() const;

private:
    void REPLACED(init)(bool isReset);
    REPLACE void init(bool isReset);
    void REPLACED(doCameraMode)();
    REPLACE void doCameraMode();

    class Mode {
    public:
        enum {
            Out = 0,
        };

    private:
        Mode();
    };

    KartBody *m_body;
    u8 _004[0x080 - 0x004];
    Mtx34 m_mtx;
    u8 _0b0[0x0e1 - 0x0b0];
    u8 m_mode;
    u8 _0e2[0x0e7 - 0x0e2];

public:
    bool m_isRearView; // Added

private:
    u8 _0e8[0x1ac - 0x0e8];
    f32 m_relAngleY;
    u8 _1b0[0x1e8 - 0x1b0];
    TVec3<f32> m_pos;
    TVec3<f32> m_target;
    u8 _200[0x294 - 0x200];
};
size_assert(KartCam, 0x294);
