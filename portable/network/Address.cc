#include "Address.hh"

bool operator==(const Address &a, const Address &b) {
    return a.address == b.address && a.port == b.port;
}

bool operator!=(const Address &a, const Address &b) {
    return !(a == b);
}
