#pragma once

extern "C" {
#include <dolphin/OSMutex.h>
}

class Mutex {
public:
    Mutex();
    ~Mutex();

    void lock();
    void unlock();

private:
    Mutex(const Mutex &);
    Mutex &operator=(const Mutex &);

    OSMutex m_mutex;
};
