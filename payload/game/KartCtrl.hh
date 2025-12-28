#pragma once

#include "game/KartBody.hh"
#include "game/KartCam.hh"
#include "game/KartGamePad.hh"
#include "game/KartPad.hh"

class KartCtrl {
public:
    KartPad *getKartPad(u32 index) const;
    KartBody *getKartBody(u32 index) const;
    KartGamePad *getDriveCont(u32 index);
    KartCam *getKartCam(u32 index) const;

    void REPLACED(dynamicsInit)(bool r4);
    REPLACE void dynamicsInit(bool r4);
    void REPLACED(dynamicsReset)();
    REPLACE void dynamicsReset();

    static KartCtrl *Instance();

private:
    KartCtrl();

    u8 _000[0x060 - 0x000];
    KartPad *m_kartPads[8];
    u8 _080[0x0a0 - 0x080];
    KartBody *m_kartBodies[8];
    u8 _0c0[0x200 - 0x0c0];
    KartCam *m_kartCams[8];
    u8 _220[0x22c - 0x220];
    u32 m_kartCount;
    u8 _230[0x238 - 0x230];

    static KartCtrl *s_instance;
};
