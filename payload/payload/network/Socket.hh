#pragma once

#include "payload/Mutex.hh"

#include <common/Array.hh>
#include <common/ios/Resource.hh>
extern "C" {
#include <dolphin/OSThread.h>
}

class Socket {
public:
    class Domain {
    public:
        enum {
            IPv4 = 2,
            IPv6 = 23,
        };

    private:
        Domain();
    };

    class Type {
    public:
        enum {
            TCP = 1,
            UDP = 2,
        };

    private:
        Type();
    };

    class Result {
    public:
        enum {
            Success = 0,
            Invalid = -28,
            NetworkReset = -39,
        };

    private:
        Result();
    };

    struct Address {
        Array<u8, 4> address;
        u16 port;
    };

    s32 close();
    bool ok();

    static void Init();

protected:
    class Ioctl {
    public:
        enum {
            Close = 0x03,
            FCntl = 0x05,
            Socket = 0x0f,
        };

    private:
        Ioctl();
    };

    class Ioctlv {
    public:
        enum {
            RecvFrom = 0x0c,
            SendTo = 0x0d,
        };

    private:
        Ioctlv();
    };

    Socket();
    ~Socket();

    s32 open(s32 domain, s32 type);
    s32 recvFrom(void *buffer, u32 size, Address *address);
    s32 sendTo(const void *buffer, u32 size, const Address *address);

private:
    class KdIoctl {
    public:
        enum {
            StartupSocket = 0x06,
            CleanupSocket = 0x07,
        };

    private:
        KdIoctl();
    };

    class FCntl {
    public:
        enum {
            GetFl = 3,
            SetFl = 4,
        };

    private:
        FCntl();
    };

    Socket(const Socket &);
    Socket &operator=(const Socket &);

    s32 setIsBlocking(bool isBlocking);

    s32 socket(s32 domain, s32 type, s32 protocol);
    s32 fcntl(s32 command, s32 argument);

    static void *Run(void *param);
    static s32 Ioctl(u32 ioctl, const void *input, u32 inputSize, void *output, u32 outputSize);
    static s32 Ioctlv(u32 ioctlv, u32 inputCount, u32 outputCount,
            IOS::Resource::IoctlvPair *pairs);

    alignas(0x20) Array<u8, 0x20> m_in;
    alignas(0x20) Array<u8, 0x20> m_out;
    s32 m_handle;
    u64 m_generation;

    static Array<u8, 4 * 1024> s_stack;
    static OSThread s_thread;
    static Mutex *s_mutex;
    static IOS::Resource *s_resource;
    static u64 s_generation;
};

bool operator==(const Socket::Address &a, Socket::Address &b);
bool operator!=(const Socket::Address &a, Socket::Address &b);
