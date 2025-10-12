#pragma once

#include <portable/online/ServerManager.hh>

class FakeServerManager final : public ServerManager {
public:
    FakeServerManager(const Ring<Server, MaxServerCount> &servers);
    ~FakeServerManager();

    bool isLocked() override;
    bool lock() override;
    void unlock() override;
};
