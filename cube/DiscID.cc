// clang-format off
//
// Resources:
// - https://wiibrew.org/wiki/Wii_disc#Header
//
// clang-format on

#include "DiscID.hh"

extern "C" {
#include <string.h>
}

extern "C" DiscID discID;

DiscID &DiscID::Get() {
    return discID;
}

bool DiscID::IsValid() {
    if (memcmp(discID.gameID, "GM4", strlen("GM4"))) {
        return false;
    }
    switch (discID.gameID[3]) {
    case 'P':
    case 'E':
    case 'J':
        break;
    default:
        return false;
    }

    if (memcmp(discID.makerID, "01", strlen("01"))) {
        return false;
    }

    if (discID.wiiMagic != 0) {
        return false;
    }

    if (discID.gameCubeMagic != 0xc2339f3d) {
        return false;
    }

    return true;
}
