#pragma once

#include "portable/Array.hh"
#include "portable/Optional.hh"

#include <formats/Online.hh>

struct ClientStateModeReadInfo {
    struct Mode {
        Array<u16, 4> mmrs;
        u16 playerCount;
    };

    Optional<Array<Mode, ModeIndex::Count>> modes;
};
