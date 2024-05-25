#include "UDPSocket.hh"

UDPSocket::UDPSocket() {}

UDPSocket::~UDPSocket() {}

s32 UDPSocket::open(s32 domain) {
    return Socket::open(domain, Type::UDP);
}

s32 UDPSocket::recvFrom(void *buffer, u32 size, Address &address) {
    return Socket::recvFrom(buffer, size, &address);
}

s32 UDPSocket::sendTo(const void *buffer, u32 size, const Address &address) {
    return Socket::sendTo(buffer, size, &address);
}
