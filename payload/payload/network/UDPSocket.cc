#include "UDPSocket.hh"

UDPSocket::UDPSocket() {}

UDPSocket::~UDPSocket() {}

s32 UDPSocket::open() {
    return Socket::open(SO_SOCK_DGRAM);
}

s32 UDPSocket::recvFrom(void *buffer, u32 size, SOSockAddr &address) {
    return Socket::recvFrom(buffer, size, &address);
}

s32 UDPSocket::sendTo(const void *buffer, u32 size, const SOSockAddr &address) {
    return Socket::sendTo(buffer, size, &address);
}
