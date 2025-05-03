#include "CubeDNS.hh"

#include <cube/Arena.hh>
#include <cube/Clock.hh>

extern "C" {
#include <assert.h>
}

void CubeDNS::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x20) CubeDNS;
    assert(s_instance);
}

CubeDNS *CubeDNS::Instance() {
    return s_instance;
}

CubeDNS::CubeDNS() : DNS(m_socket) {}

s64 CubeDNS::secondsToTicks(s64 seconds) {
    return Clock::SecondsToTicks(seconds);
}

s64 CubeDNS::getMonotonicTicks() {
    return Clock::GetMonotonicTicks();
}

CubeDNS *CubeDNS::s_instance = nullptr;
