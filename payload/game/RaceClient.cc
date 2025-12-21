#include "RaceClient.hh"

#include "game/ErrorViewApp.hh"
#include "game/RaceMgr.hh"

#include <payload/online/CubeClient.hh>

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
    m_writeInfo.frame = MinClientFrame + RaceMgr::Instance()->frame();
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

RaceClient::RaceClient() : m_ok(true), m_frame(0), m_clientFrame(MinClientFrame), m_drift(0) {}

RaceClient::~RaceClient() {}

bool RaceClient::clientStatePoll(const ClientStatePollReadInfo & /* readInfo */) {
    return true;
}

bool RaceClient::clientStateRace(const ClientStateRaceReadInfo &readInfo) {
    m_ok = m_ok && readInfo.ok;
    const Optional<ClientStateRaceReadInfo::Info> &info = readInfo.info;
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
    return true;
}

void RaceClient::clientStateError() {
    ErrorViewApp::Call(6);
}

RaceClient *RaceClient::s_instance = nullptr;
