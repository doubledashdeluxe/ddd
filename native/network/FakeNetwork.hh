#pragma once

#include <portable/network/Network.hh>

class FakeNetwork final : public Network {
public:
    FakeNetwork();
    ~FakeNetwork();

    bool isRunning() const override;
};
