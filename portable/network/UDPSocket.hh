#pragma once

#include "portable/network/Address.hh"

class UDPSocket {
public:
    virtual s32 open() = 0;
    virtual s32 close() = 0;
    virtual bool ok() = 0;
    virtual s32 recvFrom(void *buffer, u32 size, Address &address) = 0;
    virtual s32 sendTo(const void *buffer, u32 size, const Address &address) = 0;
};
