#pragma once

#include <common/Types.hh>

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
