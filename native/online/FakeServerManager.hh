#pragma once

#include <portable/online/ServerManager.hh>

class FakeServerManager final : public ServerManager {
public:
    FakeServerManager(const Ring<Server, MaxServerCount> &servers);
    ~FakeServerManager();

    virtual bool isLocked() override;
    virtual bool lock() override;
    virtual void unlock() override;
};
