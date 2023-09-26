#pragma once

#include "payload/Mutex.hh"

template <typename T>
class Lock;

class NoInterrupts {
private:
    NoInterrupts();
};

template <>
class Lock<NoInterrupts> {
public:
    Lock();
    ~Lock();

private:
    Lock(const Lock &);
    Lock &operator=(const Lock &);

    BOOL m_isr;
};

template <>
class Lock<Mutex> {
public:
    Lock(Mutex &mutex);
    ~Lock();

private:
    Lock(const Lock &);
    Lock &operator=(const Lock &);

    Mutex &m_mutex;
};
