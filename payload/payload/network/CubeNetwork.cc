#include "CubeNetwork.hh"

extern "C" {
#include <dolphin/IPRoute.h>
}

bool CubeNetwork::isRunning() const {
    return SOIsRunning();
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
