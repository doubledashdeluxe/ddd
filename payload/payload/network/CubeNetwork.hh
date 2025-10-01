#pragma once

extern "C" {
#include <dolphin/IPSocket.h>
}
#include <portable/network/Network.hh>

class CubeNetwork : public Network {
public:
    bool isRunning() const override;
    const char *name() const override;
    u32 address() const override;

    void ensureStarted(SOConfig &config);
    void ensureStopped();
    u64 generation() const;

    static CubeNetwork &Instance();

private:
    CubeNetwork();

    u64 m_generation;

    static CubeNetwork s_instance;
};
