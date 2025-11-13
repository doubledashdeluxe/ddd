#pragma once

#include <portable/Array.hh>
#include <portable/online/Kart.hh>

class OnlineInfo {
public:
    void reset();

    static OnlineInfo &Instance();

    Array<u32, 4> m_profileIndices;
    Array<Array<char, 4>, 4> m_names;
    Array<Kart, 4> m_localKarts;
    u32 m_serverIndex;
    u32 m_roomType;
    u32 m_modeIndex;
    bool m_isHost;
    u32 m_roomCounter;
    u64 m_roomCode;
    Array<Kart, MaxRoomKartCount> m_karts;
    bool m_isFFA;
    u8 m_teamCount;
    u8 m_maxTeamSize;
    Array<u8, MaxRoomKartCount> m_teams;
    bool m_hasIDs;
    Array<Array<u32, 2>, 4> m_characterIDs;
    Array<u32, 4> m_kartIDs;

private:
    OnlineInfo();

    static OnlineInfo s_instance;
};
