#include "FakeDNS.hh"

FakeDNS::FakeDNS(UDPSocket &socket) : DNS(socket) {}

FakeDNS::~FakeDNS() = default;

s64 FakeDNS::secondsToTicks(s64 seconds) {
    return seconds;
}

s64 FakeDNS::getMonotonicTicks() {
    return m_ticks++;
}
