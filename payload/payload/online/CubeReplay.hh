#pragma once

#include "payload/Mutex.hh"

extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <portable/Array.hh>
#include <portable/Ring.hh>
#include <portable/online/Replay.hh>
#include <portable/online/ReplayManager.hh>

class CubeReplay : public Replay {
public:
    CubeReplay(const ReplayManager::Replay &replay, u32 clientIndex);
    ~CubeReplay() override;
    bool read(u8 *buffer, u32 &size) override;
    void seek(u32 offset) override;

private:
    void read();

    static void *Read(void *param);

    bool m_reading;
    Mutex m_mutex;
    Ring<u8, 8 * 1024> m_buffer;
    OSMessageQueue m_queue;
    Array<OSMessage, 1> m_messages;
    Array<u8, 8 * 1024> m_stack;
    OSThread m_thread;
};
