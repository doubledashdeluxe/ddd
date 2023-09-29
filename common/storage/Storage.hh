#pragma once

#include "common/Array.hh"

#ifdef PAYLOAD
extern "C" {
#include <dolphin/OSMessage.h>
#include <dolphin/OSThread.h>
}
#include <payload/Mutex.hh>
#endif

class Storage {
public:
    static void Init();

protected:
    Storage();

    void notify();
    void remove();
    void add();

    virtual void poll() = 0;

private:
    ~Storage();

#ifdef PAYLOAD
    static void *Poll(void *param);
#endif

    Storage *m_next;
#ifdef PAYLOAD
    Mutex m_mutex;
#else
    u8 _08[0x20 - 0x08];
#endif

    static Storage *s_head;
#ifdef PAYLOAD
    static OSMessageQueue s_queue;
    static Array<OSMessage, 1> s_messages;
#endif
};
static_assert(sizeof(Storage) == 0x20);
