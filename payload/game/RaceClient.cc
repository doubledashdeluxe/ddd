#include "RaceClient.hh"

#include "game/ErrorViewApp.hh"
#include "game/KartCtrl.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceInfo.hh"
#include "game/RaceMgr.hh"

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

Optional<s32> RaceClient::drift() const {
    if (m_drifts.empty()) {
        return Optional<s32>();
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

void RaceClient::read() {
    CubeClient::Instance()->read(*this);
}

void RaceClient::write() {
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    const KartCtrl *kartCtrl = KartCtrl::Instance();
    u32 frame = MinClientFrame + RaceMgr::Instance()->frame();
    for (u32 i = 0; i < kartCount; i++) {
        Ring<KartState, 30> &kartStates = m_kartStates[i];
        // We only need one state per frame.
        if (kartStates.empty() || kartStates.back()->frame < frame) {
            if (kartStates.full()) {
                kartStates.popFront();
            }
            kartStates.pushBack();
        }
        KartState &kartState = *kartStates.back();
        kartState.frame = frame;
        const KartBody *kartBody = kartCtrl->getKartBody(i);
        kartState.pos = kartBody->m_pos;
        kartState.angle = atan2(kartBody->m_mtx[0][2], kartBody->m_mtx[0][0]);
        kartState.vel = kartBody->m_vel;
    }

    m_writeInfo.frame = frame;
    for (u32 i = 0; i < m_writeInfo.kartCount; i++) {
        const KartBody *kartBody = kartCtrl->getKartBody(onlineInfo.m_localKartIndices[i]);
        f32 min = -32768.0f, max = 32768.0f;
        Vec3f pos = kartBody->m_pos;
        f32 posScale = 1.0f / 4.0f; // Between -131072 and 131072
        pos.x = floor(pos.x * posScale);
        pos.y = floor(pos.y * posScale);
        pos.z = floor(pos.z * posScale);
        assert(pos.x >= min && pos.x < max);
        assert(pos.y >= min && pos.y < max);
        assert(pos.z >= min && pos.z < max);
        m_writeInfo.karts[i].posX = pos.x;
        m_writeInfo.karts[i].posY = pos.y;
        m_writeInfo.karts[i].posZ = pos.z;
        f32 angle = atan2(kartBody->m_mtx[0][2], kartBody->m_mtx[0][0]);
        f32 angleScale = 128.0f / M_PI; // Between -π and π
        angle = floor(angle * angleScale);
        m_writeInfo.karts[i].angle = static_cast<s32>(angle);
        Vec3f vel = kartBody->m_vel;
        f32 velScale = 32.0f; // Between -1024 and 1024
        vel.x = floor(vel.x * velScale);
        vel.y = floor(vel.y * velScale);
        vel.z = floor(vel.z * velScale);
        assert(vel.x >= min && vel.x < max);
        assert(vel.y >= min && vel.y < max);
        assert(vel.z >= min && vel.z < max);
        m_writeInfo.karts[i].velX = vel.x;
        m_writeInfo.karts[i].velY = vel.y;
        m_writeInfo.karts[i].velZ = vel.z;
    }
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

RaceClient::RaceClient() : m_ok(true), m_frame(0), m_clientFrame(MinClientFrame), m_drift(0) {
    for (u32 i = 0; i < m_kartDiffs.count(); i++) {
        m_kartDiffs[i].pos = Vec3f(0.0f, 0.0f, 0.0f);
        m_kartDiffs[i].angle = 0.0f;
        m_kartDiffs[i].vel = Vec3f(0.0f, 0.0f, 0.0f);
    }
    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    m_writeInfo.kartCount = onlineInfo.m_spectating ? 0 : raceInfo.getStatusCount();
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

    for (u32 i = 0; i < info->frame - m_frame; i++) {
        if (m_drifts.full()) {
            m_drift -= *m_drifts.front();
            m_drifts.popFront();
        }
        s32 drift = info->clientFrame - info->frame;
        m_drift += drift;
        m_drifts.pushBack(drift);
    }
    m_frame = info->frame;
    m_clientFrame = info->clientFrame;

    const OnlineInfo &onlineInfo = OnlineInfo::Instance();
    const RaceInfo &raceInfo = RaceInfo::Instance();
    u32 kartCount = raceInfo.getKartCount();
    for (u32 i = 0, j = 0; i < kartCount && j < info->kartCount; i++) {
        if (!(info->kartFlags & 1 << i)) {
            continue;
        }
        const ReadInfo::Kart &kart = info->karts[j++];
        if (onlineInfo.m_karts[i].local) {
            continue;
        }
        Ring<KartState, 30> &kartStates = m_kartStates[i];
        const KartState *kartState = nullptr;
        for (u32 i = kartStates.count(); i > 0; i--) {
            if (kartStates[i - 1].frame == kart.frame) {
                kartState = &kartStates[i - 1];
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
        KartDiff &kartDiff = m_kartDiffs[i];
        TVec3<f32> &posDiff = kartDiff.pos;
        f32 posScale = 4.0f;
        posDiff.x = Convert(kart.posX, posScale);
        posDiff.y = Convert(kart.posY, posScale);
        posDiff.z = Convert(kart.posZ, posScale);
        posDiff -= kartState->pos;
        posDiff.x = TruncateDiff(posDiff.x, posScale);
        posDiff.y = TruncateDiff(posDiff.y, posScale);
        posDiff.z = TruncateDiff(posDiff.z, posScale);
        f32 angleScale = M_PI / 128.0f;
        kartDiff.angle = Convert(kart.angle, angleScale);
        kartDiff.angle -= kartState->angle;
        // We have to keep the angle in range because the correction is interpolated.
        if (kartDiff.angle < -M_PI) {
            kartDiff.angle += 2.0f * M_PI;
        }
        if (kartDiff.angle >= M_PI) {
            kartDiff.angle -= 2.0f * M_PI;
        }
        kartDiff.angle = TruncateDiff(kartDiff.angle, angleScale);
        TVec3<f32> &velDiff = kartDiff.vel;
        f32 velScale = 1.0f / 32.0f;
        velDiff.x = Convert(kart.velX, velScale);
        velDiff.y = Convert(kart.velY, velScale);
        velDiff.z = Convert(kart.velZ, velScale);
        velDiff -= kartState->vel;
        velDiff.x = TruncateDiff(velDiff.x, velScale);
        velDiff.y = TruncateDiff(velDiff.y, velScale);
        velDiff.z = TruncateDiff(velDiff.z, velScale);
        // Each state should only be processed once.
        kartStates.popFront();
    }
    const KartCtrl *kartCtrl = KartCtrl::Instance();
    for (u32 i = 0; i < kartCount; i++) {
        if (onlineInfo.m_karts[i].local) {
            continue;
        }
        KartDiff &kartDiff = m_kartDiffs[i];
        // When oscillations happen, interpolation can help reduce their amplitude over time, yet
        // we use a high factor to keep things responsive. This also makes movements a bit more
        // natural by smoothing them a bit.
        f32 t = 0.8f;
        KartBody *kartBody = kartCtrl->getKartBody(i);
        TVec3<f32> posDiff = t * kartDiff.pos;
        kartBody->m_pos += posDiff;
        kartBody->m_mtx[0][3] += posDiff.x;
        kartBody->m_mtx[1][3] += posDiff.y;
        kartBody->m_mtx[2][3] += posDiff.z;
        kartBody->m_bodyMtx[0][3] += posDiff.x;
        kartBody->m_bodyMtx[1][3] += posDiff.y;
        kartBody->m_bodyMtx[2][3] += posDiff.z;
        kartDiff.pos -= posDiff;
        f32 angleDiff = t * kartDiff.angle;
        Mtx34 mtx;
        PSMTXRotRad(mtx, 'y', angleDiff);
        PSMTXConcat(kartBody->m_mtx, mtx, kartBody->m_mtx);
        PSMTXConcat(kartBody->m_bodyMtx, mtx, kartBody->m_bodyMtx);
        kartDiff.angle -= angleDiff;
        TVec3<f32> velDiff = t * kartDiff.vel;
        kartBody->m_vel += velDiff;
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

f32 RaceClient::Convert(s32 value, f32 scale) {
    // We need to cancel the 0.5 error introduced by the floor operation.
    return (value + 0.5f) * scale;
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

RaceClient *RaceClient::s_instance = nullptr;
