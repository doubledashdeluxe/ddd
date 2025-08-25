#include "RaceMode.hh"

const char *RaceMode::IconTextureName(u32 raceMode) {
    switch (raceMode) {
    case TA:
        return "Cup_Pict_TA.bti";
    case VS:
        return "Cup_Pict_LAN.bti";
    case Balloon:
        return "Cup_Pict_Balloon.bti";
    case Bomb:
        return "Cup_Pict_Bomb.bti";
    case Escape:
        return "Cup_Pict_Shine.bti";
    default:
        return nullptr;
    }
}

const char *RaceMode::NameTextureName(u32 raceMode) {
    switch (raceMode) {
    case TA:
        return "Entry_Time_Attack.bti";
    case VS:
        return "Entry_Versus.bti";
    case Balloon:
        return "Mozi_Battle1.bti";
    case Bomb:
        return "Mozi_Battle3.bti";
    case Escape:
        return "Mozi_Battle2.bti";
    default:
        return nullptr;
    }
}
