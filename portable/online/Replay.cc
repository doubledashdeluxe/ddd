#include "Replay.hh"

const ReplayManager::Replay &Replay::replay() const {
    return m_replay;
}

u32 Replay::clientIndex() const {
    return m_clientIndex;
}

const ReplayManager::Client *Replay::client() const {
    if (m_clientIndex < m_replay.clients.count()) {
        return &m_replay.clients[m_clientIndex];
    }

    return nullptr;
}

bool Replay::ok() const {
    return m_ok;
}

Replay::Replay(const ReplayManager::Replay &replay, u32 clientIndex)
    : m_replay(replay)
    , m_clientIndex(clientIndex)
    , m_ok(true) {}
