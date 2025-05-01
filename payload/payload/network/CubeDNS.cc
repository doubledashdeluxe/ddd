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

s32 CubeDNS::recvFrom(void *buffer, u32 size, u32 &address) {
    SOSockAddr addr;
    addr.len = sizeof(addr);
    s32 result = m_socket.recvFrom(buffer, size, addr);
    if (result >= 0) {
        if (addr.port != 53) {
            return SO_EAGAIN;
        }
        address = addr.addr;
    }
    return result;
}

s32 CubeDNS::sendTo(const void *buffer, u32 size, u32 address) {
    SOSockAddr addr;
    addr.len = sizeof(addr);
    addr.family = AF_INET;
    addr.port = 53;
    addr.addr = address;
    return m_socket.sendTo(buffer, size, addr);
}

s64 CubeDNS::secondsToTicks(s64 seconds) {
    return Clock::SecondsToTicks(seconds);
}

s64 CubeDNS::getMonotonicTicks() {
    return Clock::GetMonotonicTicks();
}

CubeDNS *CubeDNS::s_instance = nullptr;
