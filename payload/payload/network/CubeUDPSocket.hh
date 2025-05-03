#pragma once

#include "payload/network/CubeSocket.hh"

#include <portable/network/UDPSocket.hh>

class CubeUDPSocket
    : public UDPSocket
    , private CubeSocket {
public:
    CubeUDPSocket();
    ~CubeUDPSocket();

    s32 open() override;
    s32 close() override;
    bool ok() override;
    s32 recvFrom(void *buffer, u32 size, Address &address) override;
    s32 sendTo(const void *buffer, u32 size, const Address &address) override;
};
