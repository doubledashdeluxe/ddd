#pragma once

extern "C" {
#include <dolphin/IPSocket.h>
}

class Socket {
public:
    s32 close();
    bool ok();

    static bool IsRunning();
    static void EnsureStarted(SOConfig &config);
    static void EnsureStopped();

protected:
    Socket();
    ~Socket();

    s32 open(s32 type);
    s32 recvFrom(void *buffer, u32 size, SOSockAddr *address);
    s32 sendTo(const void *buffer, u32 size, const SOSockAddr *address);

private:
    Socket(const Socket &);
    Socket &operator=(const Socket &);

    s32 setIsBlocking(bool isBlocking);

    s32 fcntl(s32 command, s32 argument);

    s32 m_socket;
    u64 m_generation;

    static u64 s_generation;
};
