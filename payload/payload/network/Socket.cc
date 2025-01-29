#include "Socket.hh"

extern "C" {
#include <dolphin/IPRoute.h>
}

s32 Socket::close() {
    if (!ok()) {
        return SO_SUCCESS;
    }

    return SOClose(m_socket);
}

bool Socket::ok() {
    return m_generation == s_generation && m_socket >= 0;
}

bool Socket::IsRunning() {
    return SOIsRunning();
}

void Socket::EnsureStarted(SOConfig &config) {
    if (SOIsRunning()) {
        if (IPGetConfigError(nullptr) == 0) {
            return;
        }
        SOCleanup();
    }

    SOStartup(&config);
    s_generation++;
}

void Socket::EnsureStopped() {
    if (SOIsRunning()) {
        SOCleanup();
    }
}

Socket::Socket() : m_socket(SO_EINVAL), m_generation(0) {}

Socket::~Socket() {
    close();
}

s32 Socket::open(s32 type) {
    close();

    m_generation = s_generation;

    m_socket = SOSocket(AF_INET, type, 0);
    if (m_socket < 0) {
        return m_socket;
    }

    s32 result = setIsBlocking(false);
    if (result < 0) {
        close();
        return result;
    }

    return m_socket;
}

s32 Socket::recvFrom(void *buffer, u32 size, SOSockAddr *address) {
    if (m_generation != s_generation) {
        return SO_ENETRESET;
    }

    return SORecvFrom(m_socket, buffer, size, 0, address);
}

s32 Socket::sendTo(const void *buffer, u32 size, const SOSockAddr *address) {
    if (m_generation != s_generation) {
        return SO_ENETRESET;
    }

    return SOSendTo(m_socket, buffer, size, 0, address);
}

s32 Socket::setIsBlocking(bool isBlocking) {
    s32 flags = fcntl(SO_F_GETFL, 0);
    flags &= ~(1 << 2);
    flags |= (!isBlocking << 2);
    return fcntl(SO_F_SETFL, flags);
}

s32 Socket::fcntl(s32 command, s32 argument) {
    if (m_generation != s_generation) {
        return SO_ENETRESET;
    }

    return SOFcntl(m_socket, command, argument);
}

u64 Socket::s_generation = 1;

bool operator==(const SOSockAddr &a, const SOSockAddr &b) {
    return a.len == b.len && a.family == b.family && a.port && b.port && a.addr == b.addr;
}

bool operator!=(const SOSockAddr &a, const SOSockAddr &b) {
    return !(a == b);
}
