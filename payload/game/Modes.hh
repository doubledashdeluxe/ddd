#pragma once

#include "game/RaceMode.hh"

#include <formats/Online.hh>

extern "C" {
#include <assert.h>
}

static_assert(ModeIndexCount == ModeIndex::Count);
const u32 Modes[ModeIndexCount] = {
        RaceMode::GP,
        RaceMode::Balloon,
        RaceMode::Escape,
        RaceMode::Bomb,
        RaceMode::GP,
};

const char *const ModeIconTextureNames[ModeIndexCount] = {
        "Cup_Pict_LAN.bti",
        "Cup_Pict_Balloon.bti",
        "Cup_Pict_Shine.bti",
        "Cup_Pict_Bomb.bti",
        "Cup_Pict_TA.bti",
};

const char *const ModeNameTextureNames[ModeIndexCount] = {
        "Entry_Versus.bti",
        "Mozi_Battle1.bti",
        "Mozi_Battle2.bti",
        "Mozi_Battle3.bti",
        "Entry_Time_Attack.bti",
};
