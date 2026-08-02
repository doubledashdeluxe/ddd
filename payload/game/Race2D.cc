#include "Race2D.hh"

#include "game/J2DManager.hh"
#include "game/Kart2DCommon.hh"
#include "game/KartCtrl.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceApp.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMgr.hh"
#include "game/RaceMode.hh"
#include "game/SequenceInfo.hh"
#include "game/SiUtil.hh"
#include "game/System.hh"

extern "C" {
#include <dolphin/GXAttr.h>
#include <dolphin/GXTev.h>
#include <dolphin/GXTransform.h>
}
#include <portable/Algorithm.hh>

extern "C" {
#include <math.h>
}

void Race2D::init() {
    if (!RaceApp::Instance()) {
        setup();
    }

    REPLACED(init)();

    if (!SequenceInfo::Instance().m_isOnline) {
        return;
    }

    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    u32 consoleCount = raceInfo.getConsoleCount();
    for (u32 i = 0; i < consoleCount * 2; i++) {
        for (u32 j = 0; j < 2; j++) {
            for (u32 k = 0; k < 3; k++) {
                m_playerNamePictures[i][j][k]->m_isVisible = true;
            }
        }
    }
    if (onlineInfo.m_isDuel) {
        for (u32 i = 0; i < kartCount; i++) {
            for (u32 j = 0; j < 2; j++) {
                m_characterIndications[i].characters[j].windowPicture->m_isVisible = false;
                m_characterIndications[i].characters[j].iconPicture->m_isVisible = false;
            }
            m_characterIndications[i].rankPicture->m_isVisible = false;
        }
    }
}

void Race2D::setMinimapConfig(const MinimapConfig &minimapConfig) {
    m_minimapConfig = minimapConfig;
}

void Race2D::drawPlayerMark() {
    if (!SequenceInfo::Instance().m_isOnline) {
        REPLACED(drawPlayerMark)();
        return;
    }

    if (!m_isVisible) {
        return;
    }

    m_graphContext->setViewport();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    for (u32 i = 0; i < 3; i++) {
        GXSetVtxAttrFmt(i, GX_VA_POS, GX_POS_XY, GX_F32, 0);
        GXSetVtxAttrFmt(i, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
        GXSetVtxAttrFmt(i, GX_VA_TEX0, GX_TEX_ST, GX_U8, i == 1 ? 0 : 7);
    }
    for (u32 i = 0; i < 2; i++) {
        GXSetTevColorOp(i, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
        GXSetTevAlphaOp(i, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
        GXSetTexCoordGen2(i, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
    }
    GXSetCurrentMtx(0);
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 statusCount = raceInfo.getStatusCount();
    u32 kartCount = raceInfo.getKartCount();
    f32 scale = statusCount > 2 ? 0.75f : statusCount == 2 ? 0.85f : 1.0f;
    const RaceMgr *raceMgr = RaceMgr::Instance();
    const KartCtrl *kartCtrl = KartCtrl::Instance();
    for (u32 i = 0; i < statusCount; i++) {
        const Scissor &scissor = m_scissors[i];
        GXSetScissor(scissor.x, scissor.y, scissor.w, scissor.h);
        for (u32 j = 0; j < kartCount; j++) {
            u32 kartIndex = m_playerMarkIndices[i][j];
            if (!onlineInfo.m_spectating && J2DManager::KartStatus(kartIndex) == i) {
                continue;
            }
            if (!raceMgr->raceDrawer()->kartDrawer(kartIndex)->isVisible()) {
                continue;
            }
            if (kartCtrl->getKartCam(i)->hasJumped()) {
                continue;
            }
            K2DPicture *picture = m_playerMarkPictures[i][kartIndex];
            const TBox2<f32> &box = picture->getBox();
            const Vec2f &pos = m_playerMarkPositions[i][kartIndex];
            TVec2<f32> size = scale * (box.end - box.start);
            f32 x = pos.x;
            f32 y = pos.y - size.y;
            picture->drawK2D(x, y, size.x, size.y, true);
            u32 playerCount = onlineInfo.m_karts[kartIndex].playerCount;
            for (u32 k = 0; k < playerCount; k++) {
                for (u32 l = 0; l < 3; l++) {
                    K2DPicture *charPicture = m_playerNamePictures[kartIndex][k][l];
                    charPicture->m_alpha = picture->m_alpha;
                    TVec2<f32> charSize = 14.0f / 64.0f * size;
                    f32 charX = x + (26.0f + 12.0f * l) / 64.0f * size.x;
                    f32 charY = y + (29.0f - 12.0f * (playerCount - k)) / 64.0f * size.y;
                    charPicture->drawK2D(charX, charY, charSize.x, charSize.y, true);
                }
            }
        }
    }
    u32 x = System::Get2DScisX();
    u32 y = System::Get2DScisY();
    u32 w = System::Get2DScisW();
    u32 h = System::Get2DScisH();
    GXSetScissor(x, y, w, h);
}

void Race2D::drawCourse() {
    REPLACED(drawCourse)();

    if (!m_isVisible) {
        return;
    }

    if (SequenceInfo::Instance().m_isOnline && OnlineInfo::Instance().m_spectating) {
        m_canNotSaveG2D->draw();
    }
}

void Race2D::calc() {
    REPLACED(calc)();

    if (!m_isVisible) {
        return;
    }

    if (SequenceInfo::Instance().m_isOnline && OnlineInfo::Instance().m_spectating) {
        m_canNotSaveG2D->calc();
    }
}

Race2D *Race2D::Instance() {
    return s_instance;
}

GXColor Race2D::GetPlayerNumberColor(u32 index) {
    return s_playerNumberColors[index];
}

bool Race2D::PlayerMarkIndexComparator::operator()(const u32 &a, const u32 &b) {
    return depths[a] > depths[b];
}

void Race2D::setup() {
    bool isOnline = SequenceInfo::Instance().m_isOnline;
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    u32 consoleCount = raceInfo.getConsoleCount();
    u32 statusCount = raceInfo.getStatusCount();

    if (isOnline) {
        if (onlineInfo.m_spectating) {
            m_canNotSaveG2D = new CanNotSaveG2D(JKRHeap::GetCurrentHeap());
            m_canNotSaveG2D->setupOnline();
        }

        for (u32 i = consoleCount * 2; i < 8; i++) {
            for (u32 j = 0; j < 2; j++) {
                for (u32 k = 0; k < 3; k++) {
                    m_playerNamePictures[i][j][k] = new K2DPicture;
                }
            }
        }
    }

    m_raceScreen->setHasARTrans(false, true);
    for (u32 i = 1; i <= consoleCount; i++) {
        m_raceScreen->search("itm%ubs", i)->setHasARShift(false, false);
        m_raceScreen->search("itm%ubu", i)->setHasARShift(false, false);
        m_raceScreen->search("itm%ufs", i)->setHasARShift(false, false);
        m_raceScreen->search("itm%ufu", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ub", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ubb", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ubh1", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ubh2", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%uf", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ufb", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ufh1", i)->setHasARShift(false, false);
        m_raceScreen->search("slot%ufh2", i)->setHasARShift(false, false);
    }
    if (consoleCount <= 2) {
        for (u32 i = 1; i <= consoleCount; i++) {
            for (u32 j = 0; j < 8; j++) {
                m_raceScreen->search("time%u%u", i, j)->setHasARShiftRight(true, false);
            }
            for (u32 j = 1; j <= 2; j++) {
                m_raceScreen->search("time%ut%u", i, j)->setHasARShiftRight(true, false);
            }
        }
    } else {
        m_raceScreen->search("course")->setHasARShift(false, false);
        m_maskScreen->setHasARTrans(false, true);
        m_maskScreen->setHasARShift(false, true);
        m_maskScreen->setHasARScale(false, true);
        m_maskScreen->setHasARTexCoords(false, true);
    }
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 8; j++) {
            m_playerMarkPictures[i][j]->setHasARTrans(false, false);
            m_playerMarkPictures[i][j]->setHasARShift(false, false);
        }
    }
    if (raceInfo.m_raceMode == RaceMode::Escape) {
        for (u32 i = 0; i < consoleCount; i++) {
            m_shineIndicationScreens[i]->setHasARTrans(false, true);
        }
    }
    for (u32 i = 0; i < consoleCount; i++) {
        for (u32 j = 0; j < 2; j++) {
            m_specialItemPictures[i][j]->setHasARTrans(false, false);
            m_specialItemPictures[i][j]->setHasARShift(false, false);
        }
    }
    if (isOnline) {
        for (u32 i = 0; i < 8; i++) {
            for (u32 j = 0; j < 2; j++) {
                for (u32 k = 0; k < 3; k++) {
                    m_playerNamePictures[i][j][k]->setHasARTrans(false, false);
                    m_playerNamePictures[i][j][k]->setHasARShift(false, false);
                }
            }
        }
    }

    if (isOnline) {
        Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
        GXColor black = {0, 0, 0, 0};
        for (u32 i = 0; i < kartCount; i++) {
            u32 colorIndex = onlineInfo.colorIndex(i);
            GXColor white = s_playerNumberColors[colorIndex];
            for (u32 j = 0; j < 4; j++) {
                m_playerMarkPictures[j][i]->changeTexture("PlayerNumber.bti", 0);
                m_playerMarkPictures[j][i]->setCornerColor(white);
            }
            for (u32 j = 0; j < 2; j++) {
                const Array<char, 4> &name = onlineInfo.m_karts[i].players[j].name;
                for (u32 k = 0; k < 3; k++) {
                    ResTIMG *texture = kart2DCommon->getAsciiTexture(name[k]);
                    m_playerNamePictures[i][j][k]->changeTexture(texture, 0);
                    m_playerNamePictures[i][j][k]->setBlackWhite(black, white);
                }
            }
        }
        for (u32 i = 0; i < kartCount; i++) {
            Array<char, 32> name;
            snprintf(name.values(), name.count(), "LANMap_Icon_Player%" PRIu32 ".bti", i + 1);
            m_playerPictures[i]->changeTexture(name.values(), 0);
            u32 colorIndex = onlineInfo.colorIndex(i);
            GXColor color = s_playerNumberColors[colorIndex];
            m_playerPictures[i]->setCornerColor(color);
        }
        if (statusCount >= 1 && statusCount <= 2) {
            for (u32 i = 0; i < kartCount; i++) {
                if (onlineInfo.m_karts[i].local) {
                    continue;
                }
                for (u32 j = 0; j < 2; j++) {
                    K2DPicture *picture = m_characterIndications[i].characters[j].windowPicture;
                    picture->changeTexture("chara_Window1.bti", 0);
                }
            }
        }
    }
}

void Race2D::anmTA(s32 status) {
    if (SequenceInfo::Instance().m_isOnline && OnlineInfo::Instance().m_spectating) {
        m_lapTimes[0].lapFrame = 246;
    }

    REPLACED(anmTA)(status);
}

void Race2D::calcPlayerMark() {
    if (!SequenceInfo::Instance().m_isOnline) {
        REPLACED(calcPlayerMark)();
        return;
    }

    if (!m_isVisible) {
        return;
    }

    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 statusCount = raceInfo.getStatusCount();
    u32 kartCount = raceInfo.getKartCount();
    const RaceMgr *raceMgr = RaceMgr::Instance();
    const KartCtrl *kartCtrl = KartCtrl::Instance();
    for (u32 i = 0; i < statusCount; i++) {
        const Scissor &scissor = m_scissors[i];
        PlayerMarkIndexComparator comparator;
        for (u32 j = 0; j < kartCount; j++) {
            m_playerMarkIndices[i][j] = j;
            const Mtx34 &kartMtx = raceMgr->kartLoader(j)->bodyModel().baseTRMtx();
            Vec3f kartPos = Vec3f(kartMtx[0][3], kartMtx[1][3], kartMtx[2][3]);
            Vec2f &pos = m_playerMarkPositions[i][j];
            comparator.depths[j] = SiUtil::GetScreenPos(i, kartPos, pos);
            if (!onlineInfo.m_spectating && J2DManager::KartStatus(j) == i) {
                continue;
            }
            u8 &alpha = m_playerMarkPictures[i][j]->m_alpha;
            if (kartCtrl->getKartCam(i)->hasJumped()) {
                alpha = 0;
            } else {
                bool isVisible = true;
                isVisible = isVisible && comparator.depths[j] >= 0;
                isVisible = isVisible && pos.x >= scissor.x;
                isVisible = isVisible && pos.x < scissor.x + scissor.w;
                isVisible = isVisible && pos.y >= scissor.y;
                isVisible = isVisible && pos.y < scissor.y + scissor.h;
                if (isVisible) {
                    ZCaptureMgr *zCaptureMgr = raceMgr->raceDrawer()->zCaptureMgr();
                    s32 depth = zCaptureMgr->getZValue(j, i);
                    zCaptureMgr->setPosition(j, i, pos.x, pos.y);
                    isVisible = depth >= comparator.depths[j];
                }
                alpha = isVisible ? Min<u8>(alpha, 247) + 8 : Max<u8>(alpha, 4) - 4;
            }
        }
        Sort(m_playerMarkIndices[i], kartCount, comparator);
    }
}

void Race2D::getMapPos(s32 r4, const Vec3f &pos, Vec2f &mapPos) {
    REPLACED(getMapPos)(r4, pos, mapPos);

    s16 consoleCount = RaceInfo::Instance().getConsoleCount();
    if (consoleCount <= 2) {
        if (mapPos.x >= 0.5f * 608.0f) {
            mapPos.x += J2DPane::GetARShift();
        } else {
            mapPos.x -= J2DPane::GetARShift();
        }
    }
}

void Race2D::getItemInfo(s32 r4, s32 r5, s32 r6, f32 &x, f32 &y, f32 &scale) {
    REPLACED(getItemInfo)(r4, r5, r6, x, y, scale);

    if (x >= 0.5f * 608.0f) {
        x += J2DPane::GetARShift();
    } else {
        x -= J2DPane::GetARShift();
    }
    x -= 0.5f * 608.0f;
    x *= J2DPane::GetARScale();
    x += 0.5f * 608.0f;
}

void Race2D::getCharacterColor(s32 kartIndex, s32 characterIndex, s32 frame, GXColor &windowColor,
        GXColor &iconColor, u8 &iconAlpha) {
    // clang-format off
    REPLACED(getCharacterColor)(kartIndex, characterIndex, frame, windowColor, iconColor,
            iconAlpha);
    // clang-format on

    if (!SequenceInfo::Instance().m_isOnline) {
        return;
    }

    u32 colorIndex = OnlineInfo::Instance().colorIndex(kartIndex);
    windowColor = s_playerNumberColors[colorIndex];

    f32 t = 0.75f;
    u32 statusCount = RaceInfo::Instance().getStatusCount();
    if (J2DManager::KartStatus(kartIndex) < statusCount) {
        t = fabs((20.0f - m_characterIndicationColorFrame) * (1.0f / 20.0f));
        t = t * t * (1.5f - t) + 0.5f;
    }
    windowColor.r *= t;
    windowColor.g *= t;
    windowColor.b *= t;
}

void Race2D::getStartCharPos(s32 frame, s32 index, f32 &f1) {
    REPLACED(getStartCharPos)(frame, index, f1);

    if (f1 >= 0.5f * 608.0f) {
        f1 += J2DPane::GetARShift();
    } else {
        f1 -= J2DPane::GetARShift();
    }
}

void Race2D::CalcLap() {
    bool isOnline = SequenceInfo::Instance().m_isOnline;
    if (isOnline && OnlineInfo::Instance().m_spectating) {
        u32 kartIndex = J2DManager::StatusKart(0);
        KartChecker *kartChecker = RaceMgr::Instance()->kartChecker(kartIndex);
        s_preLap[0] = kartChecker->lap();
    }

    if (isOnline && RaceMgr::Instance()->raceDirector()->raceEnd()) {
        u32 statusCount = RaceInfo::Instance().getStatusCount();
        for (u32 i = 0; i < statusCount; i++) {
            u32 kartIndex = J2DManager::StatusKart(i);
            KartChecker *kartChecker = RaceMgr::Instance()->kartChecker(kartIndex);
            if (!kartChecker->raceEnd()) {
                return;
            }
        }
    }

    REPLACED(CalcLap)();
}
