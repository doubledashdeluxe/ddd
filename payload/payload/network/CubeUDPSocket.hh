#pragma once

#include "payload/network/CubeSocket.hh"

#include <portable/network/UDPSocket.hh>

#ifdef __CWCC__
class CubeUDPSocket
    : public UDPSocket
    , private CubeSocket {
#else
class CubeUDPSocket final
    : public UDPSocket
    , private CubeSocket {
#endif
public:
    CubeUDPSocket();
    ~CubeUDPSocket();

    s32 open() override;
    s32 close() override;
    bool ok() override;
    s32 recvFrom(void *buffer, u32 size, Address &address) override;
    s32 sendTo(const void *buffer, u32 size, const Address &address) override;
};
