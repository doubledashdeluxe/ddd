#pragma once

#include "portable/online/ReplayManager.hh"

class Replay {
public:
    virtual ~Replay() {}
    virtual bool read(u8 *buffer, u32 &size) = 0;
    virtual void seek(u32 offset) = 0;

    const ReplayManager::Replay &replay() const;
    u32 clientIndex() const;
    const ReplayManager::Client *client() const;
    bool ok() const;

protected:
    Replay(const ReplayManager::Replay &replay, u32 clientIndex);

    const ReplayManager::Replay &m_replay;
    u32 m_clientIndex;
    bool m_ok;
};
