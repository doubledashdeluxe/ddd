#pragma once

extern "C" {
#include <dolphin/IPSocket.h>
}

class CubeNetwork {
public:
    bool isRunning() const;
    void ensureStarted(SOConfig &config);
    void ensureStopped();
    u64 generation() const;

    static CubeNetwork &Instance();

private:
    CubeNetwork();

    u64 m_generation;

    static CubeNetwork s_instance;
};
