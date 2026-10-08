#include "OnlineInfo.hh"

#include "game/SequenceInfo.hh"

u32 OnlineInfo::colorIndex(u32 kartIndex) const {
    return m_isFFA ? kartIndex : m_teams[kartIndex];
}

void OnlineInfo::reset() {
    m_isReplay = false;
    m_hasIDs = false;
}

void OnlineInfo::setLocalKarts() {
    const SequenceInfo &sequenceInfo = SequenceInfo::Instance();
    u32 playerCount = sequenceInfo.m_padCount;
    u32 tandemCount = playerCount - sequenceInfo.m_statusCount;
    for (u32 i = 0; i < sequenceInfo.m_statusCount; i++) {
        Kart &kart = m_localKarts[i];
        kart.local = true;
        if (i < tandemCount) {
            kart.playerCount = 2;
            kart.players[0].index = i / 2 + 0;
            kart.players[1].index = i / 2 + 1;
        } else {
            kart.playerCount = 1;
            kart.players[0].index = i + tandemCount;
        }
        for (u32 j = 0; j < kart.players.count(); j++) {
            Player &player = kart.players[j];
            if (j < kart.playerCount) {
                player.name = m_names[player.index];
            } else {
                player.index = UINT8_MAX;
                player.name = "   ";
            }
        }
    }
}

OnlineInfo &OnlineInfo::Instance() {
    return s_instance;
}

OnlineInfo::OnlineInfo() : m_roomCounter(0) {
    reset();
}

OnlineInfo OnlineInfo::s_instance;
