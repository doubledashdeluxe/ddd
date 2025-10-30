#pragma once

#include "game/RaceMode.hh"

#include <formats/Online.hh>

extern "C" {
#include <assert.h>
}

static_assert(ModeIndexCount == ModeIndex::Count);
const u32 Modes[ModeIndexCount] = {
        RaceMode::VS,
        RaceMode::Balloon,
        RaceMode::Escape,
        RaceMode::Bomb,
        RaceMode::TA,
};
