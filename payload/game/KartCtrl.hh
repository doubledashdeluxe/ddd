#pragma once

#include "game/KartAnime.hh"
#include "game/KartBody.hh"
#include "game/KartCam.hh"
#include "game/KartGamePad.hh"
#include "game/KartPad.hh"

class KartCtrl {
public:
    KartGamePad *getKartGamePad(u32 kartIndex, u32 playerIndex) const;
    KartPad *getKartPad(u32 kartIndex) const;
    KartAnime *getKartAnime(u32 kartIndex) const;
    KartBody *getKartBody(u32 kartIndex) const;
    KartGamePad *getDriveCont(u32 kartIndex) const;
    KartGamePad *getCoDriveCont(u32 kartIndex) const;
    KartCam *getKartCam(u32 kartIndex) const;

    void REPLACED(dynamicsInit)(bool r4);
    REPLACE void dynamicsInit(bool r4);
    void REPLACED(dynamicsReset)();
    REPLACE void dynamicsReset();

    static KartCtrl *Instance();

private:
    KartCtrl();

    u8 _000[0x020 - 0x000];
    KartGamePad *m_kartGamePads[8][2];
    KartPad *m_kartPads[8];
    KartAnime *m_kartAnimes[8];
    KartBody *m_kartBodies[8];
    u8 _0c0[0x200 - 0x0c0];
    KartCam *m_kartCams[8];
    u8 _220[0x22c - 0x220];
    u32 m_kartCount;
    u8 _230[0x238 - 0x230];

    static KartCtrl *s_instance;
};
