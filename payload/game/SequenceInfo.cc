#include "SequenceInfo.hh"

#include "game/Modes.hh"
#include "game/OnlineInfo.hh"
#include "game/RaceInfo.hh"

u32 SequenceInfo::getBattleMode() const {
    return m_battleMode;
}

const char *SequenceInfo::modeIconTextureName() const {
    if (m_isOnline) {
        u32 modeIndex = OnlineInfo::Instance().m_modeIndex;
        return ModeIconTextureNames[modeIndex];
    } else {
        u32 raceMode = RaceInfo::Instance().m_raceMode;
        return RaceMode::IconTextureName(raceMode);
    }
}

const char *SequenceInfo::modeNameTextureName() const {
    if (m_isOnline) {
        u32 modeIndex = OnlineInfo::Instance().m_modeIndex;
        return ModeNameTextureNames[modeIndex];
    } else {
        u32 raceMode = RaceInfo::Instance().m_raceMode;
        return RaceMode::NameTextureName(raceMode);
    }
}

void SequenceInfo::setClrGPCourse() {
    if (!m_isOnline) {
        REPLACED(setClrGPCourse)();
        return;
    }

    OnlineInfo::Instance().m_matchIndex++;
}

SequenceInfo &SequenceInfo::Instance() {
    return s_instance;
}
