#include "FakeNetwork.hh"

FakeNetwork::FakeNetwork() = default;

FakeNetwork::~FakeNetwork() = default;

bool FakeNetwork::isRunning() const {
    return true;
}

const char *FakeNetwork::name() const {
    return "Fake";
}

u32 FakeNetwork::address() const {
    return 10 << 24 | 10 << 16 | 10 << 8 | 10 << 0;
}
