#pragma once

#include "portable/Types.hh"

struct Address {
    u32 address;
    u16 port;
};

bool operator==(const Address &a, const Address &b);
bool operator!=(const Address &a, const Address &b);
