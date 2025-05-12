#include "FakeServerManager.hh"

FakeServerManager::FakeServerManager(const Ring<Server, MaxServerCount> &servers)
    : ServerManager(servers) {}

FakeServerManager::~FakeServerManager() = default;

bool FakeServerManager::isLocked() {
    return true;
}

bool FakeServerManager::lock() {
    return true;
}

void FakeServerManager::unlock() {}
