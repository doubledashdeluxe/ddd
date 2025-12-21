#pragma once

#include "portable/Optional.hh"

struct ClientStateRaceReadInfo {
    struct Info {
        u16 frame;
        u16 clientFrame;
    };

    bool ok;
    Optional<Info> info;
};
