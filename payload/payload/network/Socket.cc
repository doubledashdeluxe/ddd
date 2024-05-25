#include "Socket.hh"

#include "payload/Lock.hh"

#include <common/Arena.hh>
#include <common/Bytes.hh>
#include <common/Clock.hh>
#include <common/Memory.hh>

extern "C" {
#include <assert.h>
}

s32 Socket::close() {
    Lock<Mutex> lock(*s_mutex);
    if (!ok()) {
        return Result::Success;
    }

    Bytes::WriteBE<s32>(m_in.values(), 0x00, m_handle);
    return Ioctl(Ioctl::Close, m_in.values(), 0x04, nullptr, 0);
}

bool Socket::ok() {
    Lock<Mutex> lock(*s_mutex);
    return m_generation == s_generation && m_handle >= 0;
}

void Socket::Init() {
    s_mutex = new (MEM1Arena::Instance(), 0x4) Mutex;
    assert(s_mutex);

    s_resource = new (MEM1Arena::Instance(), 0x4) IOS::Resource("/dev/net/ip/top", IOS::Mode::None);
    assert(s_resource && s_resource->ok());

    OSCreateThread(&s_thread, Run, nullptr, s_stack.values() + s_stack.count(), s_stack.count(), 24,
            0);
    OSResumeThread(&s_thread);
}

Socket::Socket() : m_handle(Result::Invalid), m_generation(0) {}

Socket::~Socket() {
    close();
}

s32 Socket::open(s32 domain, s32 type) {
    Lock<Mutex> lock(*s_mutex);
    m_generation = s_generation;

    close();

    s32 result = socket(domain, type, 0);
    if (result < 0) {
        return result;
    }

    return setIsBlocking(false);
}

s32 Socket::recvFrom(void *buffer, u32 size, Address *address) {
    assert(Memory::IsMEM2(buffer) && Memory::IsAligned(buffer, 0x20));

    Lock<Mutex> lock(*s_mutex);
    if (m_generation != s_generation) {
        return Result::NetworkReset;
    }

    Bytes::WriteBE<s32>(m_in.values(), 0x00, m_handle);
    Bytes::WriteBE<u32>(m_in.values(), 0x04, 0); // flags

    if (address) {
        Bytes::WriteBE<u8>(m_out.values(), 0x00, 8); // address size
        Bytes::WriteBE<u8>(m_out.values(), 0x01, Domain::IPv4);
    }

    alignas(0x20) IOS::Resource::IoctlvPair pairs[3];
    pairs[0].data = m_in.values();
    pairs[0].size = 0x08;
    pairs[1].data = buffer;
    pairs[1].size = size;
    pairs[2].data = m_out.values();
    pairs[2].size = 0x08;
    s32 result = Ioctlv(Ioctlv::RecvFrom, 1, 2, pairs);
    if (result >= 0 && address) {
        address->port = Bytes::ReadBE<u16>(m_out.values(), 0x02);
        for (u32 i = 0; i < 4; i++) {
            address->address[i] = m_out[0x04 + i];
        }
    }
    return result;
}

s32 Socket::sendTo(const void *buffer, u32 size, const Address *address) {
    assert(Memory::IsMEM2(buffer) && Memory::IsAligned(buffer, 0x20));

    Lock<Mutex> lock(*s_mutex);
    if (m_generation != s_generation) {
        return Result::NetworkReset;
    }

    Bytes::WriteBE<s32>(m_in.values(), 0x00, m_handle);
    Bytes::WriteBE<u32>(m_in.values(), 0x04, 0); // flags
    Bytes::WriteBE<u32>(m_in.values(), 0x08, address != nullptr);
    if (address) {
        Bytes::WriteBE<u8>(m_in.values(), 0x0c, 8); // address size
        Bytes::WriteBE<u8>(m_in.values(), 0x0d, Domain::IPv4);
        Bytes::WriteBE<u16>(m_in.values(), 0x0e, address->port);
        for (u32 i = 0; i < 4; i++) {
            m_in[0x10 + i] = address->address[i];
        }
    }

    alignas(0x20) IOS::Resource::IoctlvPair pairs[2];
    pairs[0].data = const_cast<void *>(buffer);
    pairs[0].size = size;
    pairs[1].data = m_in.values();
    pairs[1].size = 0x14;
    return Ioctlv(Ioctlv::SendTo, 2, 0, pairs);
}

s32 Socket::setIsBlocking(bool isBlocking) {
    s32 flags = fcntl(FCntl::GetFl, 0);
    flags &= ~(1 << 2);
    flags |= (!isBlocking << 2);
    return fcntl(FCntl::SetFl, flags);
}

s32 Socket::socket(s32 domain, s32 type, s32 protocol) {
    Bytes::WriteBE<s32>(m_in.values(), 0x00, domain);
    Bytes::WriteBE<s32>(m_in.values(), 0x04, type);
    Bytes::WriteBE<s32>(m_in.values(), 0x08, protocol);
    m_handle = Ioctl(Ioctl::Socket, m_in.values(), 0x0c, nullptr, 0);
    return m_handle;
}

s32 Socket::fcntl(s32 command, s32 argument) {
    Lock<Mutex> lock(*s_mutex);
    if (m_generation != s_generation) {
        return Result::NetworkReset;
    }

    Bytes::WriteBE<s32>(m_in.values(), 0x00, m_handle);
    Bytes::WriteBE<s32>(m_in.values(), 0x04, command);
    Bytes::WriteBE<s32>(m_in.values(), 0x08, argument);
    return Ioctl(Ioctl::FCntl, m_in.values(), 0x0c, nullptr, 0);
}

void *Socket::Run(void * /* param */) {
    IOS::Resource kdResource("/dev/net/kd/request", IOS::Mode::None);
    assert(kdResource.ok());

    Socket socket;

    while (true) {
        {
            Lock<Mutex> lock(*s_mutex);
            s_generation++;
        }

        alignas(0x20) Array<u8, 0x20> out;
        while (kdResource.ioctl(KdIoctl::StartupSocket, nullptr, 0, out.values(), out.count()) !=
                0) {
            Clock::WaitMilliseconds(1000);
        }
        do {
            Clock::WaitMilliseconds(1000);
        } while (socket.open(Domain::IPv4, Type::UDP) >= 0);
        while (kdResource.ioctl(KdIoctl::CleanupSocket, nullptr, 0, out.values(), out.count()) !=
                0) {
            Clock::WaitMilliseconds(1000);
        }
    }
}

s32 Socket::Ioctl(u32 ioctl, const void *input, u32 inputSize, void *output, u32 outputSize) {
    return s_resource->ioctl(ioctl, input, inputSize, output, outputSize);
}

s32 Socket::Ioctlv(u32 ioctlv, u32 inputCount, u32 outputCount, IOS::Resource::IoctlvPair *pairs) {
    return s_resource->ioctlv(ioctlv, inputCount, outputCount, pairs);
}

Array<u8, 4 * 1024> Socket::s_stack;
OSThread Socket::s_thread;
Mutex *Socket::s_mutex = nullptr;
IOS::Resource *Socket::s_resource = nullptr;
u64 Socket::s_generation = 1;

bool operator==(const Socket::Address &a, Socket::Address &b) {
    return a.address == b.address && a.port == b.port;
}

bool operator!=(const Socket::Address &a, Socket::Address &b) {
    return !(a == b);
}
