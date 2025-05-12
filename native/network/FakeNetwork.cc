#include "FakeNetwork.hh"

FakeNetwork::FakeNetwork() = default;

FakeNetwork::~FakeNetwork() = default;

bool FakeNetwork::isRunning() const {
    return true;
}
