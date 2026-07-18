#include "RaceClient.hh"

#include "game/ErrorViewApp.hh"
#include "game/ItemObjMgr.hh"
#include "game/KartCtrl.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMgr.hh"

#include <jsystem/JFWDisplay.hh>
#include <payload/online/CubeClient.hh>

extern "C" {
#include <assert.h>
#include <math.h>
}

bool RaceClient::ok() const {
    return m_ok;
}

u16 RaceClient::clientFrame() const {
    return m_clientFrame;
}

u16 RaceClient::latency() const {
    return m_latency;
}

s32 RaceClient::drift() const {
    if (m_drifts.empty()) {
        return 0;
    }

    return m_drift / static_cast<s32>(m_drifts.count());
}

void RaceClient::adjustDrift(s32 adjustment) {
    if (adjustment == 0) {
        return;
    }

    m_drift += adjustment * m_drifts.count();
    for (u32 i = 0; i < m_drifts.count(); i++) {
        m_drifts[i] += adjustment;
    }
}

void RaceClient::setHasItem(u32 kartIndex, u32 characterIndex) {
    u32 frame = Frame() + 1;
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    u32 kartLocalIndex = onlineInfo.m_kartLocalIndices[kartIndex];
    u16 &itemFrame = m_writeInfo.karts[kartLocalIndex].itemFrames[characterIndex];
    if (itemFrame + 50 <= frame) {
        itemFrame = frame;
    }
}

bool RaceClient::hasItem(u32 kartIndex, u32 characterIndex) const {
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    if (onlineInfo.m_karts[kartIndex].local) {
        u32 kartLocalIndex = onlineInfo.m_kartLocalIndices[kartIndex];
        u32 frame = m_writeInfo.karts[kartLocalIndex].itemFrames[characterIndex];
        if (frame != m_kartDiffs[kartIndex].itemFrames[characterIndex]) {
            return false;
        }
    }
    return m_kartDiffs[kartIndex].itemIDs[characterIndex] != ItemID::None;
}

u32 RaceClient::itemID(u32 kartIndex, u32 characterIndex) const {
    return m_kartDiffs[kartIndex].itemIDs[characterIndex];
}

void RaceClient::pushItemEvent(u32 kartIndex, const ItemEvent &itemEvent) {
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    u32 kartLocalIndex = onlineInfo.m_kartLocalIndices[kartIndex];
    WriteInfo::Kart &kart = m_writeInfo.karts[kartLocalIndex];
    if (kart.itemEventCounter == UINT8_MAX) {
        return;
    }
    ::ItemEvent *kartItemEvent = kart.itemEvents.emplaceFront();
    if (!kartItemEvent) {
        return;
    }
    kartItemEvent->frame = itemEvent.frame;
    kartItemEvent->stickY = itemEvent.stickY;
    kartItemEvent->itemID = itemEvent.itemID;
    kartItemEvent->posX = ConvertPos(itemEvent.pos.x);
    kartItemEvent->posZ = ConvertPos(itemEvent.pos.z);
    kart.itemEventCounter++;
}

const RaceClient::ItemEvent *RaceClient::itemEvent(u32 kartIndex) const {
    return m_kartDiffs[kartIndex].itemEvents.back();
}

void RaceClient::popItemEvent(u32 kartIndex) {
    m_kartDiffs[kartIndex].itemEvents.popBack();
}

void RaceClient::calcBefore() {
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    for (u32 i = 0; i < kartCount; i++) {
        UpdateItemEvents(m_kartDiffs[i].itemEvents);
    }
}

void RaceClient::calcAfter() {
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    u32 frame = Frame();
    for (u32 i = 0; i < m_writeInfo.kartCount; i++) {
        WriteInfo::Kart &kart = m_writeInfo.karts[i];
        while (kart.inputs.count() >= frame - m_clientFrame) {
            assert(kart.inputs.popFront());
        }
        WriteInfo::Inputs *inputs = kart.inputs.emplaceBack();
        assert(inputs);
        for (u32 j = 0; j < kart.inputCount; j++) {
            PadMgr::GetPadData(onlineInfo.m_padIndices[i][j], false, &(*inputs)[j]);
        }

        UpdateItemEvents(kart.itemEvents);
    }
}

void RaceClient::read() {
    CubeClient::Instance()->read(*this);
}

void RaceClient::write() {
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    const KartCtrl *kartCtrl = KartCtrl::Instance();
    u32 frame = Frame();
    for (u32 i = 0; i < kartCount; i++) {
        Ring<KartState, 30> &kartStates = m_kartStates[i];
        // We only need one state per frame.
        if (kartStates.empty() || kartStates.back()->frame < frame) {
            if (kartStates.full()) {
                kartStates.popFront();
            }
            kartStates.emplaceBack();
        }
        KartState &kartState = *kartStates.back();
        kartState.frame = frame;
        const KartBody *kartBody = kartCtrl->getKartBody(i);
        kartState.pos = kartBody->m_pos;
        kartState.angle = atan2(kartBody->m_mtx[0][2], kartBody->m_mtx[0][0]);
        kartState.vel = kartBody->m_vel;
    }

    const RaceMgr *raceMgr = RaceMgr::Instance();
    m_writeInfo.frame = frame;
    for (u32 i = 0; i < m_writeInfo.kartCount; i++) {
        u32 kartIndex = onlineInfo.m_localKartIndices[i];
        WriteInfo::Kart &kart = m_writeInfo.karts[i];
        const KartBody *kartBody = kartCtrl->getKartBody(kartIndex);
        kart.driver = kartBody->getDriver();
        const Vec3f &pos = kartBody->m_pos;
        kart.posX = ConvertPos(pos.x);
        kart.posY = ConvertPos(pos.y);
        kart.posZ = ConvertPos(pos.z);
        f32 angle = atan2(kartBody->m_mtx[0][2], kartBody->m_mtx[0][0]);
        kart.angle = ConvertAngle(angle);
        const Vec3f &vel = kartBody->m_vel;
        kart.velX = ConvertVel(vel.x);
        kart.velZ = ConvertVel(vel.z);
        const KartChecker *kartChecker = raceMgr->kartChecker(kartIndex);
        kart.rank = kartChecker->rank() - 1;
    }
    const ItemObjMgr *itemObjMgr = ItemObjMgr::Instance();
    for (u32 i = 0; i < m_writeInfo.itemCounts.count(); i++) {
        m_writeInfo.itemCounts[i] = itemObjMgr->moveCount(i);
    }
    m_writeInfo.delayedFrames = JFWDisplay::Instance()->delayedFrames();
    CubeClient::Instance()->writeStateRace(m_writeInfo);
}

void RaceClient::Create() {
    s_instance = new RaceClient;
}

void RaceClient::Destroy() {
    delete s_instance;
    s_instance = nullptr;
}

RaceClient *RaceClient::Instance() {
    return s_instance;
}

u32 RaceClient::Frame() {
    return MinClientFrame + RaceMgr::Instance()->frame();
}

RaceClient::RaceClient()
    : m_ok(true)
    , m_serverFrame(0)
    , m_clientFrame(MinClientFrame - 1)
    , m_latency(0)
    , m_drift(0) {
    for (u32 i = 0; i < m_kartDiffs.count(); i++) {
        m_kartDiffs[i].inputs.fill(0);
        m_kartDiffs[i].driver = 0;
        m_kartDiffs[i].pos = Vec3f(0.0f, 0.0f, 0.0f);
        m_kartDiffs[i].angle = 0.0f;
        m_kartDiffs[i].vel = Vec3f(0.0f, 0.0f, 0.0f);
        m_kartDiffs[i].itemFrames.fill(MinClientFrame);
        m_kartDiffs[i].itemIDs.fill(ItemID::None);
        m_kartDiffs[i].itemEventCounter = 0;
    }
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    m_writeInfo.kartCount = onlineInfo.m_localKartCount;
    for (u32 i = 0; i < m_writeInfo.kartCount; i++) {
        u32 kartIndex = onlineInfo.m_localKartIndices[i];
        WriteInfo::Kart &kart = m_writeInfo.karts[i];
        kart.inputCount = onlineInfo.m_karts[kartIndex].playerCount;
        kart.itemFrames.fill(MinClientFrame);
        kart.itemEventCounter = 0;
    }
}

RaceClient::~RaceClient() {}

bool RaceClient::clientStatePoll(const ClientStatePollReadInfo & /* readInfo */) {
    return true;
}

bool RaceClient::clientStateRace(const ClientStateRaceReadInfo &readInfo) {
    m_ok = m_ok && readInfo.ok;
    const Optional<ReadInfo::Info> &info = readInfo.info;
    if (!info) {
        return true;
    }

    for (u32 i = 0; i < info->frame - m_serverFrame; i++) {
        if (m_drifts.full()) {
            m_drift -= *m_drifts.front();
            m_drifts.popFront();
        }
        s32 drift = info->clientFrame - info->frame;
        m_drift += drift;
        m_drifts.pushBack(drift);
    }
    m_serverFrame = info->frame;
    if (info->clientFrame > m_clientFrame) {
        m_latency = Frame() + 1 - info->clientFrame;
    }
    m_clientFrame = info->clientFrame;

    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    const KartCtrl *kartCtrl = KartCtrl::Instance();
    ItemObjMgr *itemObjMgr = ItemObjMgr::Instance();
    for (u32 i = 0, j = 0; i < kartCount && j < info->kartCount; i++) {
        if (!(info->kartFlags & 1 << i)) {
            continue;
        }
        const ReadInfo::Kart &kart = info->karts[j++];
        KartDiff &kartDiff = m_kartDiffs[i];
        if (onlineInfo.m_isDuel && !onlineInfo.m_karts[i].local) {
            for (u32 k = 0; k < KartCharacterCount; k++) {
                if (kart.itemFrames[k] >= kartDiff.itemFrames[k] + 60) {
                    itemObjMgr->startItemShuffle(i, k);
                }
            }
        }
        kartDiff.itemFrames = kart.itemFrames;
        kartDiff.itemIDs = kart.itemIDs;
        if (onlineInfo.m_karts[i].local) {
            continue;
        }
        if (kart.itemEventCounter < kartDiff.itemEventCounter) {
            continue;
        }
        u32 itemEventCount = kart.itemEventCounter - kartDiff.itemEventCounter;
        if (itemEventCount > kart.itemEventCount) {
            itemEventCount = kart.itemEventCount;
        }
        for (u32 k = itemEventCount; k > 0; k--) {
            ItemEvent *itemEvent = kartDiff.itemEvents.emplaceFront();
            if (!itemEvent) {
                break;
            }
            const ::ItemEvent &kartItemEvent = kart.itemEvents[k - 1];
            itemEvent->frame = 0;
            itemEvent->stickY = kartItemEvent.stickY;
            itemEvent->itemID = kartItemEvent.itemID;
            itemEvent->pos.x = ConvertPos(kartItemEvent.posX);
            itemEvent->pos.z = ConvertPos(kartItemEvent.posZ);
            kartDiff.itemEventCounter++;
        }
        Ring<KartState, 30> &kartStates = m_kartStates[i];
        const KartState *kartState = nullptr;
        for (u32 k = kartStates.count(); k > 0; k--) {
            if (kartStates[k - 1].frame == kart.frame) {
                kartState = &kartStates[k - 1];
                break;
            }
        }
        if (!kartState) {
            continue;
        }
        // As an optimization, we can drop older states right away.
        while (kartStates.front() != kartState) {
            kartStates.popFront();
        }
        if (kart.inputCount != onlineInfo.m_karts[i].playerCount) {
            continue;
        }
        kartDiff.inputs = kart.inputs;
        for (u32 k = 0; k < kart.inputCount; k++) {
            kartDiff.inputs[k] &= ~(1 << 7 | 1 << 6); // ~(START | Z)
        }
        kartDiff.driver = !!kart.driver;
        TVec3<f32> &posDiff = kartDiff.pos;
        posDiff.x = ConvertPos(kart.posX);
        posDiff.y = ConvertPos(kart.posY);
        posDiff.z = ConvertPos(kart.posZ);
        posDiff -= kartState->pos;
        posDiff.x = TruncatePosDiff(posDiff.x);
        posDiff.y = TruncatePosDiff(posDiff.y);
        posDiff.z = TruncatePosDiff(posDiff.z);
        kartDiff.angle = ConvertAngle(kart.angle);
        kartDiff.angle -= kartState->angle;
        // We have to keep the angle in range because the correction is interpolated.
        if (kartDiff.angle < -M_PI) {
            kartDiff.angle += 2.0f * M_PI;
        }
        if (kartDiff.angle >= M_PI) {
            kartDiff.angle -= 2.0f * M_PI;
        }
        kartDiff.angle = TruncateAngleDiff(kartDiff.angle);
        TVec3<f32> &velDiff = kartDiff.vel;
        velDiff.x = ConvertVel(kart.velX);
        velDiff.z = ConvertVel(kart.velZ);
        velDiff.x -= kartState->vel.x;
        velDiff.z -= kartState->vel.z;
        velDiff.x = TruncateVelDiff(velDiff.x);
        velDiff.z = TruncateVelDiff(velDiff.z);
        // Each state should only be processed once.
        kartStates.popFront();
    }
    u32 consoleCount = raceInfo.getConsoleCount();
    for (u32 i = 0; i < kartCount; i++) {
        if (onlineInfo.m_karts[i].local) {
            continue;
        }
        KartDiff &kartDiff = m_kartDiffs[i];
        for (u32 j = 0; j < onlineInfo.m_karts[i].playerCount; j++) {
            KartGamePad *pad = kartCtrl->getKartGamePad(i, j);
            pad->expand(kartDiff.inputs[j]);
        }
        KartBody *kartBody = kartCtrl->getKartBody(i);
        if (kartBody->getDriver() != kartDiff.driver) {
            KartGame *kartGame = kartBody->getGame();
            kartGame->m_changeTimer = Max<u8>(kartGame->m_changeTimer, 2);
        }
        // When oscillations happen, interpolation can help reduce their amplitude over time, yet
        // we use a somewhat high factor to keep things responsive. This also smoothes corrective
        // movements to make them more natural.
        f32 t = 0.4f;
        TVec3<f32> posDiff = t * kartDiff.pos;
        kartBody->m_pos += posDiff;
        kartBody->m_mtx[0][3] += posDiff.x;
        kartBody->m_mtx[1][3] += posDiff.y;
        kartBody->m_mtx[2][3] += posDiff.z;
        kartBody->m_bodyMtx[0][3] += posDiff.x;
        kartBody->m_bodyMtx[1][3] += posDiff.y;
        kartBody->m_bodyMtx[2][3] += posDiff.z;
        for (u32 j = 0; j < consoleCount; j++) {
            if (!raceInfo.isDemoKart(j)) {
                continue;
            }
            KartCam *kartCam = kartCtrl->getKartCam(j);
            if (kartCam->getBody() == kartBody) {
                kartCam->m_basePos += posDiff;
            }
        }
        kartDiff.pos -= posDiff;
        f32 angleDiff = t * kartDiff.angle;
        Mtx34 mtx;
        PSMTXRotRad(mtx, 'y', angleDiff);
        PSMTXConcat(kartBody->m_mtx, mtx, kartBody->m_mtx);
        PSMTXConcat(kartBody->m_bodyMtx, mtx, kartBody->m_bodyMtx);
        kartDiff.angle -= angleDiff;
        TVec3<f32> velDiff = t * kartDiff.vel;
        Vec3f &vel = kartBody->m_vel;
        vel += velDiff;
        f32 speedScale = kartBody->m_speedScale;
        kartBody->m_xzSpeed = speedScale * sqrt(vel.x * vel.x + vel.z * vel.z);
        kartBody->m_xyzSpeed = speedScale * sqrt(vel.x * vel.x + vel.y * vel.y + vel.z * vel.z);
        kartBody->m_bodySpeed = kartBody->m_xyzSpeed * 2.16f;
        kartBody->m_speed = kartBody->m_bodySpeed;
        kartDiff.vel -= velDiff;
        // Corrections should only be applied once even if a diff is reported for multiple frames.
        Ring<KartState, 30> &kartStates = m_kartStates[i];
        for (u32 j = 0; j < kartStates.count(); j++) {
            kartStates[j].pos += posDiff;
            kartStates[j].angle += angleDiff;
            kartStates[j].vel += velDiff;
        }
    }
    return true;
}

void RaceClient::clientStateError() {
    ErrorViewApp::Call(6);
}

f32 RaceClient::Convert(f32 value, f32 scale) {
    return floor(value * scale);
}

f32 RaceClient::Convert(s32 value, f32 scale) {
    // We need to cancel the 0.5 error introduced by the floor operation.
    return (value + 0.5f) * scale;
}

s16 RaceClient::ConvertPos(f32 value) {
    value = Convert(value, PosScale);
    f32 min = -32768.0f, max = 32768.0f;
    assert(value >= min && value < max);
    return value;
}

f32 RaceClient::ConvertPos(s16 value) {
    return Convert(static_cast<s32>(value), 1.0f / PosScale);
}

s8 RaceClient::ConvertAngle(f32 value) {
    return Convert(value, AngleScale);
}

f32 RaceClient::ConvertAngle(s8 value) {
    return Convert(static_cast<s32>(value), 1.0f / AngleScale);
}

s16 RaceClient::ConvertVel(f32 value) {
    value = Convert(value, VelScale);
    f32 min = -32768.0f, max = 32768.0f;
    assert(value >= min && value < max);
    return value;
}

f32 RaceClient::ConvertVel(s16 value) {
    return Convert(static_cast<s32>(value), 1.0f / VelScale);
}

f32 RaceClient::TruncateDiff(f32 diff, f32 scale) {
    // Corrections lower than the precision should not be kept as they are unnecessary and often
    // incorrect.
    f32 precision = scale * 0.5f;
    if (diff < -precision) {
        return diff + precision;
    }
    if (diff >= precision) {
        return diff - precision;
    }
    return 0.0f;
}

f32 RaceClient::TruncatePosDiff(f32 diff) {
    return TruncateDiff(diff, 1.0f / PosScale);
}

f32 RaceClient::TruncateAngleDiff(f32 diff) {
    return TruncateDiff(diff, 1.0f / AngleScale);
}

f32 RaceClient::TruncateVelDiff(f32 diff) {
    return TruncateDiff(diff, 1.0f / VelScale);
}

RaceClient *RaceClient::s_instance = nullptr;

const f32 RaceClient::PosScale = 1.0f / 4.0f;     // Between -131072 and 131072
const f32 RaceClient::AngleScale = 128.0f / M_PI; // Between -π and π
const f32 RaceClient::VelScale = 32.0f;           // Between -1024 and 1024
