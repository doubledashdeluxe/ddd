#pragma once

#include <portable/network/Address.hh>

class CubeSocket {
protected:
    CubeSocket();
    ~CubeSocket();

    s32 open(s32 type);
    s32 close();
    bool ok();
    s32 recvFrom(void *buffer, u32 size, Address *address);
    s32 sendTo(const void *buffer, u32 size, const Address *address);

private:
    CubeSocket(const CubeSocket &);
    CubeSocket &operator=(const CubeSocket &);

    s32 setIsBlocking(bool isBlocking);

    s32 fcntl(s32 command, s32 argument);

    s32 m_socket;
    u64 m_generation;
};
