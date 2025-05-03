#include "CubeUDPSocket.hh"

extern "C" {
#include <dolphin/IPSocket.h>
}

CubeUDPSocket::CubeUDPSocket() {}

CubeUDPSocket::~CubeUDPSocket() {}

s32 CubeUDPSocket::open() {
    return CubeSocket::open(SO_SOCK_DGRAM);
}

s32 CubeUDPSocket::close() {
    return CubeSocket::close();
}

bool CubeUDPSocket::ok() {
    return CubeSocket::ok();
}

s32 CubeUDPSocket::recvFrom(void *buffer, u32 size, Address &address) {
    return CubeSocket::recvFrom(buffer, size, &address);
}

s32 CubeUDPSocket::sendTo(const void *buffer, u32 size, const Address &address) {
    return CubeSocket::sendTo(buffer, size, &address);
}
