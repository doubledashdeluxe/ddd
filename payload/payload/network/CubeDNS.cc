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

CubeDNS::CubeDNS() {}

bool CubeDNS::ok() {
    return m_socket.ok();
}

s32 CubeDNS::open() {
    return m_socket.open();
}

s32 CubeDNS::recvFrom(void *buffer, u32 size, Address &address) {
    return m_socket.recvFrom(buffer, size, address);
}

s32 CubeDNS::sendTo(const void *buffer, u32 size, const Address &address) {
    return m_socket.sendTo(buffer, size, address);
}

s64 CubeDNS::secondsToTicks(s64 seconds) {
    return Clock::SecondsToTicks(seconds);
}

s64 CubeDNS::getMonotonicTicks() {
    return Clock::GetMonotonicTicks();
}

CubeDNS *CubeDNS::s_instance = nullptr;
