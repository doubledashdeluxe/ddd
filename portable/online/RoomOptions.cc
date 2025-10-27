#include "RoomOptions.hh"

extern "C" {
#include <string.h>
}

bool operator==(const RoomOptions &a, const RoomOptions &b) {
    return !memcmp(&a, &b, sizeof(a));
}

bool operator!=(const RoomOptions &a, const RoomOptions &b) {
    return !(a == b);
}
