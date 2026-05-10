#include "Result2D.hh"

#include "game/GameAudioMain.hh"
#include "game/J2DManager.hh"
#include "game/Kart2DCommon.hh"
#include "game/OnlineInfo.hh"
#include "game/OnlineTimer.hh"
#include "game/Race2D.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMgr.hh"
#include "game/RaceMode.hh"
#include "game/ResMgr.hh"
#include "game/RoomType.hh"
#include "game/SequenceInfo.hh"
#include "game/System.hh"

#include <jsystem/J2DAnmLoaderDataBase.hh>
#include <portable/Algorithm.hh>

extern "C" {
#include <inttypes.h>
#include <math.h>
}

Result2D::Result2D(JKRHeap *heap, Online /* online */)
    : m_hioNode(new (heap, 0) HioNode)
    , m_graphContext(System::GetJ2DOrtho())
    , m_overScreen(new (heap, 0) J2DScreen)
    , m_selectorCount(2)
    , m_gpLayoutScreen(new (heap, 0) J2DScreen)
    , m_selectorY(224.0f)
    , m_selectorX(840.0f) {
    m_hioNode->isVisible = true;

    for (u32 i = 0; i < m_selectorCount; i++) {
        m_selectorScreens[i] = new (heap, 0) J2DScreen;
    }
    for (u32 i = 0; i < Count(m_gpLineScreens); i++) {
        m_gpLineScreens[i] = new (heap, 0) J2DScreen;
    }
    for (u32 i = 0; i < Count(m_gpPointScreens); i++) {
        m_gpPointScreens[i] = new (heap, 0) J2DScreen;
    }

    JKRArchive *archive = J2DManager::Instance()->archive();
    m_overScreen->set("OverScreen.blo", archive);
    for (u32 i = 0; i < m_selectorCount; i++) {
        m_selectorScreens[i]->set("SetWordLine.blo", archive);
    }
    m_gpLayoutScreen->set("OnlineResultLayout.blo", archive);
    for (u32 i = 0; i < Count(m_gpLineScreens); i++) {
        m_gpLineScreens[i]->set("OnlineResultLine.blo", archive);
    }
    for (u32 i = 0; i < Count(m_gpPointScreens); i++) {
        m_gpPointScreens[i]->set("OnlineGetPoint.blo", archive);
    }

    for (u32 i = 0; i < m_selectorCount; i++) {
        Selector &selector = m_selectors[i];
        J2DScreen *screen = m_selectorScreens[i];
        for (u32 j = 0; j < Count(selector.leftLines); j++) {
            Setup(selector.leftLines[j], screen, "RCursBL%u", j + 1);
        }
        for (u32 j = 0; j < Count(selector.rightLines); j++) {
            Setup(selector.rightLines[j], screen, "RCursBR%u", j + 1);
        }
        Setup(selector.name, screen, "RMozi00");
        Setup(selector.leftCursor, screen, "RCurs01");
        Setup(selector.rightCursor, screen, "RCurs02");
        if (i != 0) {
            selector.leftCursor.picture->m_isVisible = false;
            selector.rightCursor.picture->m_isVisible = false;
        }
    }
    Setup(m_gpLayout.courseLine, m_gpLayoutScreen, "RCoNaBA");
    Setup(m_gpLayout.courseName, m_gpLayoutScreen, "RCoNaA");
    Setup(m_gpLayout.cupIcon, m_gpLayoutScreen, "RCuPctA");
    for (u32 i = 0; i < Count(m_gpLayout.lineYs); i++) {
        m_gpLayout.lineYs[i] = m_gpLayoutScreen->search("SetPos%02u", i + 1)->getBox().start.y;
    }
    for (u32 i = 0; i < Count(m_gpLayout.transXs); i++) {
        m_gpLayout.transXs[i] = m_gpLayoutScreen->search("RTr%02u", i + 1)->getBox().start.x;
    }
    for (u32 i = 0; i < Count(m_gpLines); i++) {
        GPLine &gpLine = m_gpLines[i];
        J2DScreen *screen = m_gpLineScreens[i];
        for (u32 j = 0; j < Count(gpLine.cursors); j++) {
            Setup(gpLine.cursors[j], screen, "RCurs%02u", j + 1);
        }
        for (u32 j = 0; j < Count(gpLine.characters); j++) {
            Setup(gpLine.characters[j].box, screen, "RChaB%02u", 2 - j);
            Setup(gpLine.characters[j].icon, screen, "RChara%02u", 2 - j);
        }
        Setup(gpLine.rank, screen, "RPos00");
        for (u32 j = 0; j < Count(gpLine.timeDigits); j++) {
            Setup(gpLine.timeDigits[j], screen, "Rtime%02u", j);
        }
        for (u32 j = 0; j < Count(gpLine.timeSeps); j++) {
            Setup(gpLine.timeSeps[j], screen, "RtimeT%02u", j + 1);
        }
        for (u32 j = 0; j < Count(gpLine.plusDigits); j++) {
            Setup(gpLine.plusDigits[j], screen, "RPlus%02u", j);
        }
        Setup(gpLine.arrow, screen, "Arrow_00");
        for (u32 j = 0; j < Count(m_playerNamePictures[i]); j++) {
            for (u32 k = 0; k < Count(m_playerNamePictures[i][j]); k++) {
                J2DPicture *picture = screen->search("PName%u%u", j, k)->downcast<J2DPicture>();
                m_playerNamePictures[i][j][k] = picture;
            }
        }
        for (u32 j = 0; j < Count(m_plusDigitPictures[i]); j++) {
            J2DPicture *picture = screen->search("RPlus%02u", j)->downcast<J2DPicture>();
            m_plusDigitPictures[i][j] = picture;
        }
    }
    for (u32 i = 0; i < Count(m_gpPointPositions); i++) {
        m_gpPointPositions[i] = m_gpLineScreens[0]->search("RPSet%02u", i + 1)->getBox().start;
    }
    for (u32 i = 0; i < Count(m_gpPoints); i++) {
        GPPoint &gpPoint = m_gpPoints[i];
        J2DScreen *screen = m_gpPointScreens[i];
        Setup(gpPoint.points, screen, "Point_00");
        for (u32 j = 0; j < Count(gpPoint.pointDigits); j++) {
            Setup(gpPoint.pointDigits[j], screen, "Point_%02u", j + 1);
        }
    }

    m_selectors[0].name.picture->changeTexture("Mozi_Continue.bti", 0);
    m_selectors[1].name.picture->changeTexture("Mozi_Quit.bti", 0);
    void *courseName = ResMgr::GetPtr(ResMgr::CourseDataID::CourseName);
    if (courseName) {
        m_gpLayout.courseName.picture->changeTexture(static_cast<ResTIMG *>(courseName), 0);
    } else {
        m_gpLayout.courseName.picture->m_isVisible = false;
    }
    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    m_gpLayout.cupIcon.picture->changeTexture(sequenceInfo.modeIconTextureName(), 0);
    const Race2D *race2D = Race2D::Instance();
    for (u32 i = 0; i < Count(m_gpLines); i++) {
        GPLine &gpLine = m_gpLines[i];
        const J2DPicture *rankPicture = race2D->rankPicture(0, i);
        gpLine.rank.picture->m_cornerColors = rankPicture->m_cornerColors;
        char name[32];
        snprintf(name, Count(name), "position1_%" PRIu32 ".bti", i + 1);
        gpLine.rank.picture->changeTexture(name, 0);
    }
    u32 roomType = OnlineInfo::Instance().m_roomType;
    if (roomType == RoomType::Worldwide) {
        for (u32 i = 0; i < Count(m_gpPoints); i++) {
            GPPoint &gpPoint = m_gpPoints[i];
            gpPoint.points.picture->changeTexture("MMR.bti", 0);
            m_gpPointScreens[i]->search("Point_04")->m_isVisible = true;
        }
    }

    J2DAnmBase *layoutAnmTransform = J2DAnmLoaderDataBase::Load("ResultLayout.bck", archive);
    J2DPane *lineItemScale = m_gpLayoutScreen->search("RAniS01");
    J2DPane *pointItemScale = m_gpLayoutScreen->search("RAniS02");
    lineItemScale->setAnimation(layoutAnmTransform);
    pointItemScale->setAnimation(layoutAnmTransform);
    for (u32 i = 0; i < 10; i++) {
        layoutAnmTransform->m_frame = i;
        m_gpLayoutScreen->animation();
        m_gpLineItemScales[i] = lineItemScale->getScale();
        m_gpPointItemScales[i] = pointItemScale->getScale();
    }
    J2DAnmBase *gpPointAnmTransform = J2DAnmLoaderDataBase::Load("OnlineGetPoint.bck", archive);
    gpPointAnmTransform->m_frame = roomType == RoomType::Worldwide;
    for (u32 i = 0; i < Count(m_gpPointScreens); i++) {
        m_gpPointScreens[i]->setAnimation(gpPointAnmTransform);
        m_gpPointScreens[i]->animation();
    }

    m_draw = &Result2D::drawOnline;
    m_setDraw = &Result2D::setDrawOnline;
    m_selector = &Result2D::selectorOnline;

    switch (RaceInfo::Instance().getRaceMode()) {
    case RaceMode::GP:
    case RaceMode::Balloon:
        m_hasOnlineTime = true;
        break;
    default:
        m_hasOnlineTime = false;
        break;
    }
    m_hasOnlineTotal = roomType == RoomType::Personal;

    OnlineTimer::Create(archive);

    init();
}

void Result2D::init() {
    m_overScreen->search("OverBack")->setHasARScale(false, false);

    REPLACED(init)();
}

void Result2D::calc(const KartGamePad *pad) {
    if (!SequenceInfo::Instance().m_isOnline) {
        REPLACED(calc)(pad);
        return;
    }

    if (!s_isVisible) {
        return;
    }

    s32 kartCount = RaceInfo::Instance().getKartCount();
    const JUTGamePad::CButton &button = pad->button();
    s32 inFrame;
    u32 nextState;
    switch (s_state) {
    case State::OnlineTime:
        calcOnlineTime();
        setOnlineTime();
        inFrame = 45;
        nextState = State::OnlineMatch;
        break;
    case State::OnlineMatch:
        calcOnlineMatch();
        setOnlineMatch();
        inFrame = 175;
        nextState = m_hasOnlineTotal ? State::OnlineTotal : State::Selector;
        break;
    case State::OnlineTotal:
        calcOnlineTotal();
        setOnlineTotal();
        inFrame = 45;
        nextState = State::Selector;
        break;
    case State::Selector:
        calcSelector();
        if (m_endFrame == 0 && m_selectorX == 0.0f && m_selectorCursorFrame == 0) {
            (this->*m_selector)(pad);
        }
        (this->*m_setDraw)();
        break;
    }
    if (s_state != State::Selector) {
        if (m_frame >= inFrame - (8 - kartCount) * 5 && m_frame < 295) {
            m_frame = inFrame;
        }
        if ((button.risingEdge() & PAD_BUTTON_A || OnlineTimer::Instance()->hasExpired()) &&
                m_frame >= inFrame && m_frame < 295) {
            m_frame = 295;
        }
        if (m_frame == 360 - (8 - kartCount) * 5) {
            s_state = nextState;
            m_frame = -1;
        }
    }

    m_frame++;
    if (m_endFrame != 0) {
        m_endFrame++;
    }
    m_overBackAlpha = Min<s32>(m_overBackAlpha + 10, 128);
    if (m_endFrame != 0) {
        m_overBackAlpha -= Min((m_endFrame - 1) * 10, m_overBackAlpha);
    }
    m_overScreen->search("OverBack")->setAlpha(m_overBackAlpha);
    m_lineColorFrame = (m_lineColorFrame + 1) % 50;
    if (m_selectorX == 0.0f && m_anmEndFrame < 30) {
        m_anmEndFrame++;
    }
    if (m_selectorCursorFrame < 8) {
        f32 t = m_selectorCursorFrame * (1.0f / 8.0f);
        m_selectorCursorCurrY = m_selectorCursorPrevY * (1.0f - t) + m_selectorCursorNextY * t;
        if (m_selectorCursorFrame != 0) {
            m_selectorCursorFrame++;
        }
    } else {
        m_selectorCursorPrevY = m_selectorCursorNextY;
        m_selectorCursorCurrY = m_selectorCursorNextY;
        m_selectorCursorFrame = 0;
    }
    m_selectorScaleFrame++;
    m_selectorScale = getSelectorScale(m_selectorScaleFrame);

    OnlineTimer::Instance()->calc();
}

void Result2D::end() {
    m_endFrame = 1;
}

void Result2D::SetGPClr() {
    if (SequenceInfo::Instance().m_isOnline) {
        switch (RaceInfo::Instance().getRaceMode()) {
        case RaceMode::GP:
        case RaceMode::Balloon:
            s_state = State::OnlineTime;
            break;
        default:
            s_state = State::OnlineMatch;
            break;
        }
        OnlineTimer::Instance()->init(30);
    } else {
        REPLACED(SetGPClr)();
    }
}

u32 Result2D::GetState() {
    return s_state;
}

u32 Result2D::GetSelector() {
    return s_selector;
}

void Result2D::GPLineModifier::modify(GPLine &gpLine) {
    for (u32 i = 0; i < Count(gpLine.cursors); i++) {
        modify(gpLine.cursors[i]);
    }
    for (u32 i = 0; i < Count(gpLine.characters); i++) {
        modify(gpLine.characters[i].box);
        modify(gpLine.characters[i].icon);
    }
    modify(gpLine.rank);
    for (u32 i = 0; i < Count(gpLine.timeDigits); i++) {
        modify(gpLine.timeDigits[i]);
    }
    for (u32 i = 0; i < Count(gpLine.timeSeps); i++) {
        modify(gpLine.timeSeps[i]);
    }
    for (u32 i = 0; i < Count(gpLine.plusDigits); i++) {
        modify(gpLine.plusDigits[i]);
    }
    modify(gpLine.arrow);
}

Result2D::GPLineScaler::GPLineScaler(GPLine &gpLine, const TVec2<f32> &scale) : m_scale(scale) {
    GPLineModifier::modify(gpLine);
}

void Result2D::GPLineScaler::modify(Data2D &data) {
    data.picture->scale(m_scale);
}

void Result2D::GPPointModifier::modify(GPPoint &gpPoint) {
    modify(gpPoint.points);
    for (u32 i = 0; i < Count(gpPoint.pointDigits); i++) {
        modify(gpPoint.pointDigits[i]);
    }
}

Result2D::GPPointScaler::GPPointScaler(GPPoint &gpPoint, const TVec2<f32> &scale) : m_scale(scale) {
    GPPointModifier::modify(gpPoint);
}

void Result2D::GPPointScaler::modify(Data2D &data) {
    data.picture->scale(m_scale);
}

void Result2D::drawOnline() {
    m_overScreen->draw(0.0f, 0.0f, m_graphContext);
    m_gpLayoutScreen->draw(0.0f, 0.0f, m_graphContext);
    u32 kartCount = RaceInfo::Instance().getKartCount();
    for (u32 i = 0; i < kartCount; i++) {
        f32 x = m_gpLayout.transXs[1] + m_gpLineXs[i];
        f32 y = m_gpLayout.lineYs[i];
        m_gpLineScreens[i]->draw(x, y, m_graphContext);
    }
    const TVec2<f32> &pointPos = m_gpPointPositions[0];
    for (u32 i = 0; i < kartCount; i++) {
        f32 x = pointPos.x + m_gpLayout.transXs[1] + m_gpLineXs[i];
        f32 y = pointPos.y + m_gpLayout.lineYs[i];
        m_gpPointScreens[i]->draw(x, y, m_graphContext);
    }
    for (u32 i = 0; i < m_selectorCount; i++) {
        f32 x = m_selectorX;
        f32 y = m_selectorY - 16.0f + i * 32.0f;
        m_selectorScreens[i]->draw(x, y, m_graphContext);
    }
    OnlineTimer::Instance()->draw(m_graphContext);
}

void Result2D::setDrawOnline() {
    setSelectorScale(s_selector == 8);
}

void Result2D::selectorOnline(const KartGamePad *pad) {
    const JUTGamePad::CButton &button = pad->button();
    if (button.repeat() & JUTGamePad::PAD_MSTICK_UP) {
        s_selector = s_selector == 8 ? 5 : 8;
        selectorCommon();
    }
    if (button.repeat() & JUTGamePad::PAD_MSTICK_DOWN) {
        s_selector = s_selector == 8 ? 5 : 8;
        selectorCommon();
    }
}

void Result2D::calcOnlineTime() {
    calcOnlineCommon(false, true);
}

void Result2D::calcOnlineMatch() {
    calcOnlineCommon(m_hasOnlineTime, m_hasOnlineTotal);
    u32 kartCount = RaceInfo::Instance().getKartCount();
    s32 frame = m_frame - 75 + (8 - kartCount) * 5;
    if (frame >= 0 && frame < 100 && frame % 10 == 9) {
        for (u32 i = 0; i < kartCount; i++) {
            if (m_pointDiffs[i] > 0) {
                m_pointDiffs[i]--;
            }
        }
        GameAudio::Main::Instance()->startSystemSe(SoundID::JA_SE_TR_SCORE_POINT_ADD);
    }
}

void Result2D::calcOnlineTotal() {
    calcOnlineCommon(true, false);
}

void Result2D::calcOnlineCommon(bool hasPrev, bool hasNext) {
    if (m_frame < 175) {
        if (!hasPrev) {
            getRaceRankLayoutAlpha(m_frame, m_gpLayoutAlpha);
        }
    } else {
        if (!hasNext) {
            getGPRankLayoutAlpha(m_frame - 130, m_gpLayoutAlpha);
        }
    }
    u32 kartCount = RaceInfo::Instance().getKartCount();
    for (u32 i = 0; i < kartCount; i++) {
        Vec2f &lineScale = m_gpScales[i].line, &pointScale = m_gpScales[i].point;
        if (m_frame < 175) {
            if (hasPrev) {
                getGPRankLineScale(m_frame + (8 - kartCount) * 5, i, lineScale, pointScale);
            } else {
                getRaceRankLinePos(m_frame + (8 - kartCount) * 5, i, m_gpLineXs[i]);
            }
        } else {
            if (hasNext) {
                getRaceRankLineScale(m_frame, i, lineScale, pointScale);
            } else {
                getGPRankLinePos(m_frame - 130 + (8 - kartCount) * 5, i, m_gpLineXs[i]);
            }
        }
        GPLineScaler(m_gpLines[i], lineScale);
        for (u32 j = 0; j < Count(m_playerNamePictures[i]); j++) {
            for (u32 k = 0; k < Count(m_playerNamePictures[i][j]); k++) {
                m_playerNamePictures[i][j][k]->scale(lineScale);
            }
        }
        for (u32 j = 2; j < Count(m_plusDigitPictures[i]); j++) {
            m_plusDigitPictures[i][j]->scale(lineScale);
        }
        GPPointScaler(m_gpPoints[i], pointScale);
        m_gpPointScreens[i]->search("Point_04")->scale(pointScale);
    }
}

void Result2D::calcSelector() {
    s32 frame = m_frame + RaceInfo::Instance().isBattle() * 40;
    getSelectorPos(frame, m_selectorX);
    if (m_endFrame != 0) {
        u8 alpha = (10 - Min<u32>(m_endFrame - 1, 10)) * 255 / 10;
        OnlineTimer::Instance()->setAlpha(alpha);
    }
}

void Result2D::setOnlineTime() {
    setOnlineCommon(false);
    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    const RaceMgr *raceMgr = RaceMgr::Instance();
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < kartCount; i++) {
        u32 kartIndex = sequenceInfo.m_raceRankedKartIndices[i];
        const KartChecker *kartChecker = raceMgr->kartChecker(kartIndex);
        const RaceTime &totalTime = kartChecker->totalTime();
        s32 minutes, seconds, milliseconds;
        totalTime.get(minutes, seconds, milliseconds);
        J2DPicture *pictures[7];
        for (u32 j = 0; j < Count(pictures); j++) {
            pictures[j] = m_gpLines[i].timeDigits[j].picture;
        }
        kart2DCommon->changeNumberTexture(milliseconds, pictures + 0, 3, true, false);
        kart2DCommon->changeNumberTexture(seconds, pictures + 3, 2, true, false);
        kart2DCommon->changeNumberTexture(minutes, pictures + 5, 2, true, false);
        m_gpPointScreens[i]->m_isVisible = false;
    }
}

void Result2D::setOnlineMatch() {
    setOnlineCommon(false);
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < kartCount; i++) {
        for (u32 j = 0; j < Count(m_gpLines[i].timeDigits); j++) {
            m_gpLines[i].timeDigits[j].picture->m_isVisible = false;
        }
        for (u32 j = 0; j < Count(m_gpLines[i].timeSeps); j++) {
            m_gpLines[i].timeSeps[j].picture->m_isVisible = false;
        }
        m_gpPointScreens[i]->m_isVisible = true;
        char plusText[5];
        s32 points[8] = {10, 8, 6, 4, 3, 2, 1, 0};
        s32 count = snprintf(plusText, Count(plusText), "%+" PRId32, points[i]);
        s32 maxCount = onlineInfo.m_roomType == RoomType::Worldwide ? 4 : 3;
        count = Clamp<s32>(count, 0, maxCount);
        u32 offset = maxCount - count;
        kart2DCommon->changeUnicodeTexture(plusText, m_plusDigitPictures[i] + offset, count);
    }
}

void Result2D::setOnlineTotal() {
    setOnlineCommon(true);
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    for (u32 i = 0; i < kartCount; i++) {
        for (u32 j = 0; j < Count(m_plusDigitPictures[i]); j++) {
            m_plusDigitPictures[i][j]->m_isVisible = false;
        }
    }
}

void Result2D::setOnlineCommon(bool isTotal) {
    m_gpLayout.courseLine.picture->setAlpha(m_gpLayoutAlpha);
    m_gpLayout.courseName.picture->setAlpha(m_gpLayoutAlpha);
    m_gpLayout.cupIcon.picture->setAlpha(m_gpLayoutAlpha);
    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    Kart2DCommon *kart2DCommon = Kart2DCommon::Instance();
    for (u32 i = 0; i < kartCount; i++) {
        J2DScreen *screen = m_gpLineScreens[i];
        u32 kartIndex = isTotal ? sequenceInfo.m_gpRankedKartIndices[i]
                                : sequenceInfo.m_raceRankedKartIndices[i];
        GXColor kartColor;
        getKartColor(kartIndex, kartColor);
        for (u32 j = 0; j < Count(m_gpLines[i].cursors); j++) {
            J2DPicture *picture = m_gpLines[i].cursors[j].picture;
            picture->setWhite(kartColor);
        }
        const Kart &kart = onlineInfo.m_karts[kartIndex];
        for (u32 j = 0; j < Count(m_gpLines[i].characters); j++) {
            GXColor boxColor;
            J2DPicture::CornerColors charColors;
            getCharacterColor(kartIndex, j, boxColor, charColors);
            J2DPicture *boxPicture = m_gpLines[i].characters[1 - j].box.picture;
            boxPicture->setWhite(boxColor);
            J2DPicture *iconPicture = m_gpLines[i].characters[1 - j].icon.picture;
            u32 characterID = raceInfo.getKartInfo(kartIndex).getCharDB(j)->id - 1;
            ResTIMG *icon = kart2DCommon->getCharacterIcon(characterID);
            iconPicture->changeTexture(icon, 0);
            const Player &player = kart.players[j];
            char prefix[32];
            snprintf(prefix, Count(prefix), "PName%" PRIu32, j);
            kart2DCommon->changeUnicodeTexture(player.name.values(), 3, *screen, prefix);
            for (u32 k = 0; k < 3; k++) {
                m_playerNamePictures[i][j][k]->m_cornerColors = charColors;
            }
        }
        u32 points = sequenceInfo.m_points[kartIndex] - m_pointDiffs[i];
        J2DPicture *pictures[4];
        for (u32 j = 0; j < Count(m_gpPoints[i].pointDigits); j++) {
            pictures[j] = m_gpPoints[i].pointDigits[j].picture;
        }
        pictures[3] = m_gpPointScreens[i]->search("Point_04")->downcast<J2DPicture>();
        kart2DCommon->changeNumberTexture(points, pictures, 4, false, false);
    }
    if (m_frame < 175) {
        OnlineTimer::Instance()->setAlpha(m_gpLayoutAlpha);
    }
}

void Result2D::getKartColor(s32 kartIndex, GXColor &color) const {
    u32 colorIndex = OnlineInfo::Instance().colorIndex(kartIndex);
    color = Race2D::GetPlayerNumberColor(colorIndex);
}

void Result2D::getCharacterColor(s32 kartIndex, s32 characterIndex, GXColor &boxColor,
        J2DPicture::CornerColors &charColors) const {
    const Kart &kart = OnlineInfo::Instance().m_karts[kartIndex];
    if (kart.local) {
        const Player &player = kart.players[Min<u32>(characterIndex, kart.playerCount - 1)];
        boxColor = Race2D::GetPlayerNumberColor(player.index);
        charColors = Race2D::GetCornerColors(player.index);
        f32 t = fabs((25.0f - m_lineColorFrame) * (1.0f / 25.0f));
        t = t * t * (1.5f - t) + 0.5f;
        boxColor.r *= t;
        boxColor.g *= t;
        boxColor.b *= t;
    } else {
        boxColor = (GXColor){170, 170, 170, 255};
        charColors.topLeft = (GXColor){255, 255, 255, 255};
        charColors.topRight = (GXColor){85, 85, 85, 255};
        charColors.bottomLeft = (GXColor){85, 85, 85, 255};
        charColors.bottomRight = (GXColor){255, 255, 255, 255};
    }
}

void Result2D::Setup(Data2D &data, J2DScreen *screen, const char *format, ...) {
    va_list vlist;
    va_start(vlist, format);
    data.picture = screen->vsearch(format, vlist)->downcast<J2DPicture>();
    va_end(vlist);
    const TBox2<f32> &box = data.picture->getBox();
    data.pos.x = (box.start.x + box.end.x) * 0.5f;
    data.pos.y = (box.start.y + box.end.y) * 0.5f;
}
