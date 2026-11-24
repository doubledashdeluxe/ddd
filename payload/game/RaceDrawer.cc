#include "RaceDrawer.hh"

#include "game/ItemObjMgr.hh"
#include "game/J2DManager.hh"
#include "game/KartCtrl.hh"
#include "game/LightMgr.hh"
#include "game/RaceInfo.hh"
#include "game/SequenceInfo.hh"

extern "C" {
#include <dolphin/GXPixel.h>
#include <dolphin/GXTexture.h>
}
#include <jsystem/J3DSys.hh>

void RaceDrawer::setup() {
    if (!SequenceInfo::Instance().isOnlineTA()) {
        return;
    }

    u32 consoleCount = RaceInfo::Instance().getConsoleCount();
    m_ghostScreens = new GhostScreen[consoleCount];
    for (u32 i = 0; i < consoleCount; i++) {
        m_ghostScreens[i].setConsole(i);
    }
}

void RaceDrawer::drawMidScene() {
    REPLACED(drawMidScene)();

    if (!SequenceInfo::Instance().isOnlineTA()) {
        return;
    }

    u32 consoleCount = RaceInfo::Instance().getConsoleCount();
    for (u32 i = 0; i < consoleCount; i++) {
        if (m_ghostScreens[0].hidden()) {
            continue;
        }

        GhostScreen &ghostScreen = m_ghostScreens[i];
        GXSetColorUpdate(false);
        GXSetAlphaUpdate(true);
        GXSetZMode(true, GX_LEQUAL, false);
        ghostScreen.captureScene();

        GXSetColorUpdate(true);
        GXSetZMode(true, GX_LEQUAL, true);
        drawGhost(i);
        J3DSys::Instance().reinitGX();
        ghostScreen.captureGhost();

        m_graphContext->setViewport();
        ghostScreen.draw();
    }
}

KartDrawer *RaceDrawer::kartDrawer(u32 index) {
    return m_kartDrawers + index;
}

ZCaptureMgr *RaceDrawer::zCaptureMgr() {
    return m_zCaptureMgr;
}

void RaceDrawer::drawGhost(u32 console) {
    J3DSys &j3dSys = J3DSys::Instance();
    KartCtrl *kartCtrl = KartCtrl::Instance();
    KartCam *kartCam = kartCtrl->getKartCam(console);
    j3dSys.setViewMtx(kartCam->mtx());
    LightMgr *lightMgr = LightMgr::Instance();
    lightMgr->draw();
    kartCtrl->drawKartInit(console);
    j3dSys.drawInit();
    j3dSys.setTexCacheRegion(GX_TEXCACHE_128K);

    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    for (u32 i = 0; i < kartCount; i++) {
        GXSetDstAlpha(true, 255 - m_kartDrawers[i].ghostAlpha());
        m_kartDrawers[i].drawKart(console, KartDrawer::Stage::Ghost);
        m_kartDrawers[i].drawDriver(console, KartDrawer::Stage::Ghost);
    }

    ItemObjMgr *itemObjMgr = ItemObjMgr::Instance();
    for (u32 i = 0; i < kartCount; i++) {
        if (J2DManager::KartStatus(i) == console) {
            continue;
        }

        GXSetDstAlpha(true, 255 - m_kartDrawers[i].ghostAlpha());
        LightObj *light = m_kartDrawers[i].light(console);
        light->draw();
        itemObjMgr->REPLACED(drawSimpleModel)(console, 1, i, light);
    }

    GXSetDstAlpha(true, 0);
}
