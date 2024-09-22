#include "ClientK.hh"

#include "payload/crypto/Random.hh"

#include <common/Arena.hh>

void ClientK::Init() {
    s_instance = new (MEM1Arena::Instance(), 0x20) Array<u8, 32>;
    Random::Get(s_instance->values(), s_instance->count());
}

const Array<u8, 32> &ClientK::Get() {
    return *s_instance;
}

Array<u8, 32> *ClientK::s_instance = nullptr;
