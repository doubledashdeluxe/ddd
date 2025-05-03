#include "CubeSocket.hh"

extern "C" {
#include <dolphin/IPRoute.h>
}

bool CubeSocket::IsRunning() {
    return SOIsRunning();
}

void CubeSocket::EnsureStarted(SOConfig &config) {
    if (SOIsRunning()) {
        if (IPGetConfigError(nullptr) == 0) {
            return;
        }
        SOCleanup();
    }

    SOStartup(&config);
    s_generation++;
}

void CubeSocket::EnsureStopped() {
    if (SOIsRunning()) {
        SOCleanup();
    }
}

CubeSocket::CubeSocket() : m_socket(SO_EINVAL), m_generation(0) {}

CubeSocket::~CubeSocket() {
    close();
}

s32 CubeSocket::open(s32 type) {
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

s32 CubeSocket::close() {
    if (!ok()) {
        return SO_SUCCESS;
    }

    return SOClose(m_socket);
}

bool CubeSocket::ok() {
    return m_generation == s_generation && m_socket >= 0;
}

s32 CubeSocket::recvFrom(void *buffer, u32 size, Address *address) {
    if (m_generation != s_generation) {
        return SO_ENETRESET;
    }

    if (!address) {
        return SORecvFrom(m_socket, buffer, size, 0, nullptr);
    }

    SOSockAddr addr;
    addr.len = sizeof(addr);
    s32 result = SORecvFrom(m_socket, buffer, size, 0, &addr);
    if (result >= 0) {
        address->address = addr.addr;
        address->port = addr.port;
    }
    return result;
}

s32 CubeSocket::sendTo(const void *buffer, u32 size, const Address *address) {
    if (m_generation != s_generation) {
        return SO_ENETRESET;
    }

    if (!address) {
        return SOSendTo(m_socket, buffer, size, 0, nullptr);
    }

    SOSockAddr addr;
    addr.len = sizeof(addr);
    addr.family = AF_INET;
    addr.port = address->port;
    addr.addr = address->address;
    return SOSendTo(m_socket, buffer, size, 0, &addr);
}

s32 CubeSocket::setIsBlocking(bool isBlocking) {
    s32 flags = fcntl(SO_F_GETFL, 0);
    flags &= ~(1 << 2);
    flags |= (!isBlocking << 2);
    return fcntl(SO_F_SETFL, flags);
}

s32 CubeSocket::fcntl(s32 command, s32 argument) {
    if (m_generation != s_generation) {
        return SO_ENETRESET;
    }

    return SOFcntl(m_socket, command, argument);
}

u64 CubeSocket::s_generation = 1;
