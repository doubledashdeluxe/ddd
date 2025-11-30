#pragma once

#include <portable/Array.hh>
#include <portable/online/Kart.hh>

class OnlineInfo {
public:
    u32 colorIndex(u32 kartIndex) const;

    void reset();

    static OnlineInfo &Instance();

    Array<u32, 4> m_profileIndices;
    Array<Array<char, 4>, 4> m_names;
    Array<Kart, 4> m_localKarts;
    u32 m_serverIndex;
    u32 m_roomType;
    u32 m_modeIndex;
    u32 m_format;
    bool m_isHost;
    u32 m_roomCounter;
    u64 m_roomCode;
    Array<Kart, MaxRoomKartCount> m_karts;
    Array<u8, MaxRoomKartCount> m_localKartIndices;
    bool m_spectating;
    bool m_isFFA;
    u8 m_teamCount;
    u8 m_maxTeamSize;
    Array<u8, MaxRoomKartCount> m_teams;
    Array<Array<u8, 2>, 4> m_padIndices;
    bool m_hasIDs;
    Array<Array<u8, 2>, 4> m_characterIDs;
    Array<u8, 4> m_kartIDs;
    bool m_hasCourseSelection;
    bool m_hasCourseShuffle;

private:
    OnlineInfo();

    static OnlineInfo s_instance;
};
