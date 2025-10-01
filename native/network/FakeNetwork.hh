#pragma once

#include <portable/network/Network.hh>

class FakeNetwork final : public Network {
public:
    FakeNetwork();
    ~FakeNetwork();

    bool isRunning() const override;
    const char *name() const override;
    u32 address() const override;
};
