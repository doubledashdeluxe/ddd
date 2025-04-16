extern "C" {
#include "IPSocket.h"

#include "dolphin/ETH.h"
#include "dolphin/IPEther.h"
#include "dolphin/OSMessage.h"
#include "dolphin/OSThread.h"
}

#include <common/Align.hh>
#include <common/Arena.hh>
#include <common/Bytes.hh>
#include <common/Clock.hh>
#include <common/Platform.hh>
#include <common/UniquePtr.hh>
#include <common/ios/Resource.hh>
#include <jsystem/JKRExpHeap.hh>
#include <jsystem/JKRHeap.hh>
#include <payload/Lock.hh>
#include <portable/Array.hh>

extern "C" {
#include <assert.h>
#include <stdarg.h>
#include <string.h>
}

class Ioctl {
public:
    enum {
        Accept = 0x01,
        Bind = 0x02,
        Close = 0x03,
        Fcntl = 0x05,
        SetSockOpt = 0x09,
        Listen = 0x0a,
        Poll = 0x0b,
        Shutdown = 0x0e,
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
        GetInterfaceOpt = 0x1c,
        SetInterfaceOpt = 0x1d,
    };

private:
    Ioctlv();
};

class KdIoctl {
public:
    enum {
        StartupSocket = 0x06,
        CleanupSocket = 0x07,
    };

private:
    KdIoctl();
};

static bool s_isVirtual = false;
static JKRHeap *s_heap;
static IOS::Resource *s_resource;
static IOS::Resource *s_kdResource;
static OSMessageQueue s_queue;
static bool s_currIsRunning = false;
static bool s_nextIsRunning = false;
static SOConfig s_config;

static void *Run(void * /* param */) {
    while (true) {
        OSReceiveMessage(&s_queue, nullptr, OS_MESSAGE_BLOCK);

        {
            Lock<NoInterrupts> lock;
            if (s_nextIsRunning == s_currIsRunning) {
                continue;
            }
        }

        alignas(0x20) Array<u8, 0x20> out;
        if (s_currIsRunning) {
            if (s_isVirtual) {
                while (s_kdResource->ioctl(KdIoctl::CleanupSocket, nullptr, 0, out.values(),
                               out.count()) < 0) {
                    Clock::WaitMilliseconds(100);
                }
            }
            s_isVirtual = false;
            REPLACED(SOCleanup)();
        } else {
            s_isVirtual = false;
            REPLACED(SOStartup)(&s_config);
            s_isVirtual = !Platform::IsGameCube() && IFDefault.mode == ETH_MODE_UNKNOWN;
            if (s_isVirtual) {
                while (s_kdResource->ioctl(KdIoctl::StartupSocket, nullptr, 0, out.values(),
                               out.count()) < 0) {
                    Clock::WaitMilliseconds(100);
                }
            }
        }

        Lock<NoInterrupts> lock;
        s_currIsRunning = !s_currIsRunning;
    }
}

void SOInit() {
    REPLACED(SOInit)();
    s_isVirtual = false;
    if (!Platform::IsGameCube()) {
        size_t heapSize = 16 * 1024;
        void *heapPtr = MEM2Arena::Instance()->alloc(heapSize, 0x4);
        s_heap = JKRExpHeap::Create(heapPtr, heapSize, JKRHeap::GetRootHeap(), false);
        s_resource = new (s_heap, 0x4) IOS::Resource("/dev/net/ip/top", IOS::Mode::None);
        assert(s_resource->ok());
        s_kdResource = new (s_heap, 0x4) IOS::Resource("/dev/net/kd/request", IOS::Mode::None);
        assert(s_kdResource->ok());
    }
    static Array<OSMessage, 1> messages;
    OSInitMessageQueue(&s_queue, messages.values(), messages.count());
    void *param = nullptr;
    alignas(0x8) static Array<u8, 4 * 1024> stack;
    static OSThread thread;
    OSCreateThread(&thread, Run, param, stack.values() + stack.count(), stack.count(), 17, 0);
    OSResumeThread(&thread);
}

s32 SOStartup(SOConfig *config) {
    Lock<NoInterrupts> lock;
    if (s_currIsRunning && s_nextIsRunning) {
        if (!memcmp(config, &s_config, sizeof(s_config))) {
            return SO_SUCCESS;
        }
        SOCleanup();
    }
    if (!s_currIsRunning && !s_nextIsRunning) {
        s_nextIsRunning = true;
        s_config = *config;
        OSSendMessage(&s_queue, nullptr, OS_MESSAGE_NOBLOCK);
    }
    return SO_ENETRESET;
}

s32 SOCleanup() {
    Lock<NoInterrupts> lock;
    s_nextIsRunning = false;
    OSSendMessage(&s_queue, nullptr, OS_MESSAGE_NOBLOCK);
    return SO_SUCCESS;
}

s32 SOSocket(s32 domain, s32 type, s32 protocol) {
    if (!s_isVirtual) {
        return REPLACED(SOSocket)(domain, type, protocol);
    }

    alignas(0x20) Array<u8, 0xc> in;
    Bytes::WriteBE<s32>(in.values(), 0x0, domain);
    Bytes::WriteBE<s32>(in.values(), 0x4, type);
    Bytes::WriteBE<s32>(in.values(), 0x8, protocol);
    return s_resource->ioctl(Ioctl::Socket, in.values(), in.count(), nullptr, 0);
}

s32 SOClose(s32 socket) {
    if (!s_isVirtual) {
        return REPLACED(SOClose)(socket);
    }

    alignas(0x20) Array<u8, 0x4> in;
    Bytes::WriteBE<s32>(in.values(), 0x0, socket);
    return s_resource->ioctl(Ioctl::Close, in.values(), in.count(), nullptr, 0);
}

s32 SOListen(s32 socket, s32 backlog) {
    if (!s_isVirtual) {
        return REPLACED(SOListen)(socket, backlog);
    }

    alignas(0x20) Array<u8, 0x8> in;
    Bytes::WriteBE<s32>(in.values(), 0x0, socket);
    Bytes::WriteBE<s32>(in.values(), 0x4, backlog);
    return s_resource->ioctl(Ioctl::Listen, in.values(), in.count(), nullptr, 0);
}

s32 SOAccept(s32 socket, SOSockAddr *address) {
    if (!s_isVirtual) {
        return REPLACED(SOAccept)(socket, address);
    }

    alignas(0x20) Array<u8, 0x4> in;
    Bytes::WriteBE<s32>(in.values(), 0x0, socket);
    alignas(0x20) Array<u8, sizeof(*address)> out;
    if (address) {
        memcpy(out.values(), address, sizeof(*address));
    }
    s32 result =
            s_resource->ioctl(Ioctl::Accept, in.values(), in.count(), out.values(), out.count());
    if (result >= 0 && address) {
        memcpy(address, out.values(), sizeof(*address));
    }
    return result;
}

s32 SOBind(s32 socket, const SOSockAddr *address) {
    if (!s_isVirtual) {
        return REPLACED(SOBind)(socket, address);
    }

    alignas(0x20) Array<u8, 0x10> in;
    Bytes::WriteBE<s32>(in.values(), 0x00, socket);
    Bytes::WriteBE<s32>(in.values(), 0x04, 1);
    memcpy(in.values() + 0x08, address, sizeof(*address));
    return s_resource->ioctl(Ioctl::Bind, in.values(), in.count(), nullptr, 0);
}

s32 SOShutdown(s32 socket, s32 how) {
    if (!s_isVirtual) {
        return REPLACED(SOShutdown)(socket, how);
    }

    alignas(0x20) Array<u8, 0x8> in;
    Bytes::WriteBE<s32>(in.values(), 0x0, socket);
    Bytes::WriteBE<s32>(in.values(), 0x4, how);
    return s_resource->ioctl(Ioctl::Shutdown, in.values(), in.count(), nullptr, 0);
}

s32 SORecvFrom(s32 socket, void *buffer, s32 length, s32 flags, SOSockAddr *address) {
    if (!s_isVirtual) {
        return REPLACED(SORecvFrom)(socket, buffer, length, flags, address);
    }

    alignas(0x20) Array<u8, 0x8> in;
    Bytes::WriteBE<s32>(in.values(), 0x0, socket);
    Bytes::WriteBE<s32>(in.values(), 0x4, flags);

    UniquePtr<u8[]> out(new (s_heap, 0x20) u8[AlignUp(0x20 + length, 0x20)]);
    if (address) {
        memcpy(out.get() + 0x00, address, sizeof(*address));
    }

    alignas(0x20) IOS::Resource::IoctlvPair pairs[3];
    pairs[0].data = in.values();
    pairs[0].size = in.count();
    pairs[1].data = out.get() + 0x20;
    pairs[1].size = length;
    pairs[2].data = address ? out.get() + 0x00 : nullptr;
    pairs[2].size = address ? sizeof(*address) : 0x0;
    s32 result = s_resource->ioctlv(Ioctlv::RecvFrom, 1, 2, pairs);
    if (result >= 0) {
        memcpy(buffer, out.get() + 0x20, length);
        if (address) {
            memcpy(address, out.get() + 0x00, sizeof(*address));
        }
    }
    return result;
}

s32 SOSendTo(s32 socket, const void *buffer, s32 length, s32 flags, const SOSockAddr *address) {
    if (!s_isVirtual) {
        return REPLACED(SOSendTo)(socket, buffer, length, flags, address);
    }

    UniquePtr<u8[]> in(new (s_heap, 0x20) u8[AlignUp(0x20 + length, 0x20)]);
    memcpy(in.get() + 0x20, buffer, length);
    Bytes::WriteBE<s32>(in.get(), 0x00, socket);
    Bytes::WriteBE<s32>(in.get(), 0x04, flags);
    Bytes::WriteBE<s32>(in.get(), 0x08, !!address);
    if (address) {
        memcpy(in.get() + 0x0c, address, sizeof(*address));
    }

    alignas(0x20) IOS::Resource::IoctlvPair pairs[2];
    pairs[0].data = in.get() + 0x20;
    pairs[0].size = length;
    pairs[1].data = in.get() + 0x00;
    pairs[1].size = 0x14;
    return s_resource->ioctlv(Ioctlv::SendTo, 2, 0, pairs);
}

s32 SOSetSockOpt(s32 socket, s32 level, s32 optname, const void *optval, s32 optlen) {
    if (!s_isVirtual) {
        return REPLACED(SOSetSockOpt)(socket, level, optname, optval, optlen);
    }

    UniquePtr<u8[]> in(new (s_heap, 0x20) u8[AlignUp(0x10 + optlen, 0x20)]);
    Bytes::WriteBE<s32>(in.get(), 0x00, socket);
    Bytes::WriteBE<s32>(in.get(), 0x04, level);
    Bytes::WriteBE<s32>(in.get(), 0x08, optname);
    Bytes::WriteBE<s32>(in.get(), 0x0c, optlen);
    memcpy(in.get() + 0x10, optval, optlen);
    return s_resource->ioctl(Ioctl::SetSockOpt, in.get(), 0x10 + optlen, nullptr, 0);
}

s32 SOFcntl(s32 socket, s32 cmd, ...) {
    s32 arg = 0;
    if (cmd == SO_F_SETFL) {
        va_list vlist;
        va_start(vlist, cmd);
        arg = va_arg(vlist, s32);
        va_end(vlist);
    }

    if (!s_isVirtual) {
        return REPLACED(SOFcntl)(socket, cmd, arg);
    }

    alignas(0x20) Array<u8, 0xc> in;
    Bytes::WriteBE<s32>(in.values(), 0x00, socket);
    Bytes::WriteBE<s32>(in.values(), 0x04, cmd);
    Bytes::WriteBE<s32>(in.values(), 0x08, arg);
    return s_resource->ioctl(Ioctl::Fcntl, in.values(), in.count(), nullptr, 0);
}

s32 SOPoll(SOPollFD *fds, u32 nfds, s64 timeout) {
    if (!s_isVirtual) {
        return REPLACED(SOPoll)(fds, nfds, timeout);
    }

    if (timeout >= 0) {
        timeout = Clock::TicksToMilliseconds(timeout);
    }

    alignas(0x20) Array<u8, 0x8> in;
    Bytes::WriteBE<s64>(in.values(), 0x0, timeout);
    UniquePtr<u8[]> out(new (s_heap, 0x20) u8[AlignUp(nfds * 0xc, 0x20)]);
    for (u32 i = 0; i < nfds; i++) {
        Bytes::WriteBE<s32>(out.get(), i * 0xc + 0x0, fds[i].fd);
        Bytes::WriteBE<s32>(out.get(), i * 0xc + 0x4, fds[i].fd < 0 ? 0 : fds[i].events);
        Bytes::WriteBE<s32>(out.get(), i * 0xc + 0x8, fds[i].fd < 0 ? 0 : fds[i].revents);
    }
    s32 result = s_resource->ioctl(Ioctl::Poll, in.values(), in.count(), out.get(), nfds * 0xc);
    if (result >= 0) {
        for (u32 i = 0; i < nfds; i++) {
            fds[i].fd = Bytes::ReadBE<s32>(out.get(), i * 0xc + 0x0);
            fds[i].events = Bytes::ReadBE<s32>(out.get(), i * 0xc + 0x4);
            fds[i].revents = Bytes::ReadBE<s32>(out.get(), i * 0xc + 0x8);
        }
    }
    return result;
}

s32 SOGetInterfaceOpt(s32 optname, void *optval, s32 *optlen) {
    if (!s_isVirtual) {
        return SO_EINVAL;
    }

    alignas(0x20) Array<u8, 0x8> in;
    Bytes::WriteBE<s32>(in.values(), 0x0, 0xfffe);
    Bytes::WriteBE<s32>(in.values(), 0x4, optname);

    UniquePtr<u8[]> out(new (s_heap, 0x20) u8[AlignUp(0x20 + *optlen, 0x20)]);
    Bytes::WriteBE<s32>(out.get(), 0x00, *optlen);

    alignas(0x20) IOS::Resource::IoctlvPair pairs[3];
    pairs[0].data = in.values();
    pairs[0].size = in.count();
    pairs[1].data = out.get() + 0x20;
    pairs[1].size = *optlen;
    pairs[2].data = out.get() + 0x00;
    pairs[2].size = 0x4;
    s32 result = s_resource->ioctlv(Ioctlv::GetInterfaceOpt, 1, 2, pairs);
    if (result >= 0) {
        s32 optlenOut = Bytes::ReadBE<s32>(out.get(), 0x00);
        if (optlenOut > *optlen) {
            result = SO_EINVAL;
        } else {
            memcpy(optval, out.get() + 0x20, optlenOut);
        }
        *optlen = optlenOut;
    }
    return result;
}

s32 SOSetInterfaceOpt(s32 optname, const void *optval, s32 optlen) {
    if (!s_isVirtual) {
        return SO_EINVAL;
    }

    UniquePtr<u8[]> in(new (s_heap, 0x20) u8[AlignUp(0x20 + optlen, 0x20)]);
    Bytes::WriteBE<s32>(in.get(), 0x00, 0xfffe);
    Bytes::WriteBE<s32>(in.get(), 0x04, optname);
    memcpy(in.get() + 0x20, optval, optlen);

    alignas(0x20) IOS::Resource::IoctlvPair pairs[2];
    pairs[0].data = in.get() + 0x00;
    pairs[0].size = 0x8;
    pairs[1].data = in.get() + 0x20;
    pairs[1].size = optlen;
    return s_resource->ioctlv(Ioctlv::SetInterfaceOpt, 2, 0, pairs);
}

bool SOIsRunning() {
    Lock<NoInterrupts> lock;
    return s_currIsRunning && s_nextIsRunning;
}

bool SOIsVirtual() {
    return s_isVirtual;
}
