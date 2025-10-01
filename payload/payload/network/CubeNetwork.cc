#include "CubeNetwork.hh"

extern "C" {
#include <dolphin/IPRoute.h>
}
#include <portable/Bytes.hh>

bool CubeNetwork::isRunning() const {
    return SOIsRunning();
}

const char *CubeNetwork::name() const {
    return SOName();
}

u32 CubeNetwork::address() const {
    u8 addr[4];
    IPGetAddr(nullptr, addr);
    return Bytes::ReadBE<u32>(addr, 0);
}

void CubeNetwork::ensureStarted(SOConfig &config) {
    if (SOIsRunning()) {
        if (IPGetConfigError(nullptr) == 0) {
            return;
        }
        SOCleanup();
    }

    SOStartup(&config);
    m_generation++;
}

void CubeNetwork::ensureStopped() {
    if (SOIsRunning()) {
        SOCleanup();
    }
}

u64 CubeNetwork::generation() const {
    return m_generation;
}

CubeNetwork &CubeNetwork::Instance() {
    return s_instance;
}

CubeNetwork::CubeNetwork() : m_generation(1) {}

CubeNetwork CubeNetwork::s_instance;
