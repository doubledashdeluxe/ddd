#pragma once

#include "game/RaceMode.hh"

#include <formats/Online.hh>

const u32 Modes[ModeIndex::Count] = {
        RaceMode::VS,
        RaceMode::Balloon,
        RaceMode::Escape,
        RaceMode::Bomb,
        RaceMode::TA,
};
