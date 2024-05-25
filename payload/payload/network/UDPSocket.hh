#pragma once

#include "payload/network/Socket.hh"

class UDPSocket : public Socket {
public:
    UDPSocket();
    ~UDPSocket();

    s32 open(s32 domain);
    s32 recvFrom(void *buffer, u32 size, Address &address);
    s32 sendTo(const void *buffer, u32 size, const Address &address);
};
