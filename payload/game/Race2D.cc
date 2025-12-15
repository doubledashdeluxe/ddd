#include "Race2D.hh"

#include "game/J2DManager.hh"
#include "game/Kart2DCommon.hh"
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

    bool isOnline = SequenceInfo::Instance().m_isOnline;
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 consoleCount = raceInfo.getConsoleCount();

    if (isOnline) {
        for (u32 i = 0; i < consoleCount * 2; i++) {
            for (u32 j = 0; j < 2; j++) {
                for (u32 k = 0; k < 3; k++) {
                    m_playerNamePictures[i][j][k]->m_isVisible = true;
                }
            }
        }
    }
}

void Race2D::setMinimapConfig(const MinimapConfig &minimapConfig) {
    m_minimapConfig = minimapConfig;
}

Race2D *Race2D::Instance() {
    return s_instance;
}

GXColor Race2D::GetPlayerNumberColor(u32 index) {
    return s_playerNumberColors[index];
}

bool Race2D::KartIndexComparator::operator()(const u32 &a, const u32 &b) {
    return race2D.m_characterIndications[a].rank > race2D.m_characterIndications[b].rank;
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
        if (consoleCount == 2 && raceInfo.getRaceMode() == RaceMode::VS) {
            for (u32 i = 0; i < 8; i++) {
                m_consoles[0].rankPositions[i].y += 23.0f;
                m_consoles[1].rankPositions[i].y -= 23.0f;
            }
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
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 statusCount = raceInfo.getStatusCount();
    u32 kartCount = raceInfo.getKartCount();
    f32 scale = statusCount > 2 ? 0.75f : statusCount == 2 ? 0.85f : 1.0f;
    RaceMgr *raceMgr = RaceMgr::Instance();
    for (u32 i = 0; i < statusCount; i++) {
        const Scissor &scissor = m_scissors[i];
        GXSetScissor(scissor.x, scissor.y, scissor.w, scissor.h);
        for (u32 j = 0; j < kartCount; j++) {
            u32 kartIndex = m_playerMarkIndices[i][j];
            if (J2DManager::KartStatus(kartIndex) == i) {
                continue;
            }
            if (!raceMgr->raceDrawer()->kartDrawer(kartIndex)->isVisible()) {
                continue;
            }
            K2DPicture *picture = m_playerMarkPictures[i][kartIndex];
            const TBox2<f32> &box = picture->getBox();
            const Vec2f &pos = m_playerMarkPositions[i][kartIndex];
            TVec2<f32> size = scale * (box.end - box.start);
            f32 x = pos.x;
            f32 y = pos.y - size.y;
            picture->drawK2D(x, y, size.x, size.y, true);
            u32 playerCount = OnlineInfo::Instance().m_karts[kartIndex].playerCount;
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

void Race2D::draw() {
    REPLACED(draw)();

    if (!SequenceInfo::Instance().m_isOnline) {
        return;
    }

    if (!m_isVisible) {
        return;
    }

    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 statusCount = raceInfo.getStatusCount();
    if (statusCount < 1 || statusCount > 2 || !raceInfo.isRace()) {
        return;
    }

    f32 scale;
    getStartScaleB(m_frame, scale);
    s32 goalAnmFrame = J2DManager::Instance()->goalAnmFrame();
    f32 goalX;
    getGoalLapTimePos(goalAnmFrame, 0, goalX);
    for (u32 i = 0; i < 10; i++) {
        f32 startX;
        getStartLapTimePos(m_frame, i, startX);
        for (u32 j = 0; j < statusCount; j++) {
            K2DPicture *picture = m_timePictures[j][i];
            const TBox2<f32> &box = picture->getBox();
            const Vec2f &pos = m_consoles[j].timePositions[i];
            TVec2<f32> size = scale * (box.end - box.start);
            f32 x = pos.x - 0.5f * size.x + startX + goalX;
            f32 y = pos.y - 0.5f * size.y;
            picture->drawK2D(x, y, size.x, size.y, true);
        }
    }
    for (u32 i = 0; i < 9; i++) {
        getGoalLapTimePos(goalAnmFrame, i, goalX);
        for (u32 j = 0; j < statusCount; j++) {
            for (u32 k = 0; k < 11; k++) {
                if (i != 0 && k == 10) {
                    continue;
                }
                K2DPicture *picture = m_lapTimePictures[j][i][k];
                const TBox2<f32> &box = picture->getBox();
                const Vec2f &pos = m_consoles[j].lapTimePositions[i][k];
                f32 x = pos.x + goalX;
                f32 y = pos.y;
                TVec2<f32> size = box.end - box.start;
                picture->drawK2D(x, y, size.x, size.y, true);
            }
        }
    }
}

void Race2D::drawCourse() {
    REPLACED(drawCourse)();

    if (!SequenceInfo::Instance().m_isOnline) {
        return;
    }

    if (!m_isVisible) {
        return;
    }

    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    u32 statusCount = raceInfo.getStatusCount();
    if (statusCount < 1 || statusCount > 2 || !raceInfo.isRace()) {
        return;
    }

    f32 scale;
    getStartScaleB(m_frame, scale);
    s32 goalAnmFrame = J2DManager::Instance()->goalAnmFrame();
    u32 kartIndices[8];
    for (u32 i = 0; i < kartCount; i++) {
        kartIndices[i] = i;
    }
    KartIndexComparator comparator = {*this};
    Sort(kartIndices, kartCount, comparator);
    for (u32 i = 0; i < kartCount; i++) {
        u32 kartIndex = kartIndices[i];
        f32 startX;
        getStartCharPos(m_frame, kartCount - 1 - kartIndex, startX);
        f32 goalX;
        getGoalCharPos(goalAnmFrame, kartCount - 1 - kartIndex, goalX);
        const CharacterIndication &indication = m_characterIndications[kartIndex];
        const CharacterIndicationAnm &anm = m_characterIndicationAnms[kartIndex];
        const Vec2f &pos = indication.pos;
        f32 characterX, characterY, characterScale;
        getCharacterInfo(anm.characterIndices[0], anm.frame, characterX, characterY,
                characterScale);
        u8 characterIndices[2];
        characterIndices[0] = characterScale > 0.9f;
        characterIndices[1] = characterScale <= 0.9f;
        u32 statusIndex = J2DManager::KartStatus(kartIndex);
        f32 statusScale = statusIndex < statusCount ? 1.2f : 1.0f;
        f32 kartScale = statusScale * getThunderCharScale(s_thunderAnm[kartIndex]);
        for (u32 j = 0; j < 2; j++) {
            u32 characterIndex = characterIndices[j];
            getCharacterInfo(anm.characterIndices[characterIndex], anm.frame, characterX,
                    characterY, characterScale);
            GXColor windowColor, iconColor;
            u8 iconAlpha;
            getCharacterColor(kartIndex, anm.characterIndices[characterIndex], anm.frame,
                    windowColor, iconColor, iconAlpha);
            f32 angle = s_spinRotate[kartIndex];
            K2DPicture *windowPicture = indication.characters[characterIndex].windowPicture;
            windowPicture->setWhite(windowColor);
            windowPicture->setAlpha(indication.alpha);
            const TBox2<f32> &windowBox = windowPicture->getBox();
            TVec2<f32> windowSize =
                    scale * kartScale * characterScale * (windowBox.end - windowBox.start);
            TVec2<f32> windowCenter = 0.5f * windowSize;
            windowPicture->rotate(windowCenter.x, windowCenter.y, 'z', angle);
            f32 windowX = pos.x + characterX - windowCenter.x + startX + goalX;
            f32 windowY = pos.y + characterY - windowCenter.y;
            windowPicture->drawK2D(windowX, windowY, windowSize.x, windowSize.y, true);
            K2DPicture *iconPicture = indication.characters[characterIndex].iconPicture;
            iconPicture->setWhite(iconColor);
            iconPicture->setAlpha((iconAlpha * indication.alpha) >> 8);
            const TBox2<f32> &iconBox = iconPicture->getBox();
            TVec2<f32> iconSize =
                    scale * kartScale * characterScale * (iconBox.end - iconBox.start);
            TVec2<f32> iconCenter = 0.5f * iconSize;
            iconPicture->rotate(iconCenter.x, iconCenter.y, 'z', angle);
            f32 sin = ::sin(angle * -(M_PI / 180.0f));
            f32 cos = ::cos(angle * -(M_PI / 180.0f));
            TVec2<f32> iconPos = characterScale * m_characterIndicationIconPos;
            iconPos.set(iconPos.x * cos - iconPos.y * sin, iconPos.x * sin + iconPos.y * cos);
            f32 iconX = pos.x + iconPos.x + characterX - iconCenter.x + startX + goalX;
            f32 iconY = pos.y + iconPos.y + characterY - iconCenter.y;
            iconPicture->drawK2D(iconX, iconY, iconSize.x, iconSize.y, true);
        }
    }
    for (u32 i = 0; i < kartCount; i++) {
        u32 kartIndex = kartIndices[i];
        f32 startX;
        getStartCharPos(m_frame, kartCount - 1 - kartIndex, startX);
        f32 goalX;
        getGoalCharPos(goalAnmFrame, kartCount - 1 - kartIndex, goalX);
        const CharacterIndication &indication = m_characterIndications[kartIndex];
        const Vec2f &pos = indication.pos;
        u32 statusIndex = J2DManager::KartStatus(kartIndex);
        f32 statusScale = statusIndex < statusCount ? 1.2f : 1.0f;
        K2DPicture *rankPicture = indication.rankPicture;
        rankPicture->setAlpha(indication.alpha);
        const TBox2<f32> &rankBox = rankPicture->getBox();
        TVec2<f32> rankSize = scale * statusScale * (rankBox.end - rankBox.start);
        TVec2<f32> rankCenter = 0.5f * rankSize;
        f32 rankX = m_characterIndicationRankPos.x + pos.x - rankCenter.x + startX + goalX;
        f32 rankY = m_characterIndicationRankPos.y + pos.y - rankCenter.y;
        rankPicture->drawK2D(rankX, rankY, rankSize.x, rankSize.y, true);
    }
}

void Race2D::calc() {
    REPLACED(calc)();

    if (!SequenceInfo::Instance().m_isOnline) {
        return;
    }

    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 statusCount = raceInfo.getStatusCount();
    if (statusCount < 1 || statusCount > 2 || !raceInfo.isRace()) {
        return;
    }

    RaceMgr *raceMgr = RaceMgr::Instance();
    for (u32 i = 0; i < statusCount; i++) {
        u32 kartIndex = J2DManager::StatusKart(i);
        KartChecker *kartChecker = raceMgr->kartChecker(kartIndex);
        if (!kartChecker->raceEnd() && kartChecker->lapRenewal()) {
            K2DPicture *picture = m_timePictures[i][0];
            picture->changeTexture("im_lap1_1.bti", 0);
            for (u32 j = 0; j < 10; j++) {
                picture = m_timePictures[i][j];
                picture->m_cornerColors = m_lapTimeCornerColors;
            }
            m_lapTimes[i].lapFrame = 1;
        }
        anmTA(i);
    }
}

void Race2D::calcPlayerMark() {
    if (!SequenceInfo::Instance().m_isOnline) {
        REPLACED(calcPlayerMark)();
        return;
    }

    if (!m_isVisible) {
        return;
    }

    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 statusCount = raceInfo.getStatusCount();
    u32 kartCount = raceInfo.getKartCount();
    RaceMgr *raceMgr = RaceMgr::Instance();
    for (u32 i = 0; i < statusCount; i++) {
        const Scissor &scissor = m_scissors[i];
        PlayerMarkIndexComparator comparator;
        for (u32 j = 0; j < kartCount; j++) {
            m_playerMarkIndices[i][j] = j;
            const Mtx34 &kartMtx = raceMgr->kartLoader(j)->bodyModel().baseTRMtx();
            Vec3f kartPos = Vec3f(kartMtx[0][3], kartMtx[1][3], kartMtx[2][3]);
            Vec2f &pos = m_playerMarkPositions[i][j];
            comparator.depths[j] = SiUtil::GetScreenPos(i, kartPos, pos);
            if (J2DManager::KartStatus(j) == i) {
                continue;
            }
            bool isVisible = true;
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
            u8 &alpha = m_playerMarkPictures[i][j]->m_alpha;
            if (isVisible) {
                alpha = Min<u8>(alpha, 247) + 8;
            } else {
                alpha = Max<u8>(alpha, 4) - 4;
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
