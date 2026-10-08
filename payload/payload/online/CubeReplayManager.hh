#pragma once

#include "payload/StorageScanner.hh"

#include <formats/Online.hh>
#include <portable/Ring.hh>
#include <portable/crypto/Types.hh>
#include <portable/online/ReplayManager.hh>

const u32 MaxReplayCount = 56;

class CubeReplayManager
    : public ReplayManager
    , public StorageScanner
    , public ReplayReader<CubeReplayManager>
    , public ReplayRaceReader<CubeReplayManager>
    , public ReplayClientReader<CubeReplayManager>
    , public ClientPlayerReader<CubeReplayManager> {
public:
    void filterAndSort();

    u32 replayCount() const;
    const Replay &replay(u32 index) const;

    bool isMagicValid(u32 magic);
    void setMagic(u32 magic);
    bool isReplayVersionValid(u16 replayVersion);
    void setReplayVersion(u16 replayVersion);
    bool isReservedValid(u16 reserved);
    void setReserved(u16 reserved);
    bool isVersionCountValid(u32 versionCount);
    void setVersionCount(u32 versionCount);
    bool isVersionElementValid(u32 i0, u8 versionElement);
    void setVersionElement(u32 i0, u8 versionElement);

    bool isFrameRateValid(u8 frameRate);
    void setFrameRate(u8 frameRate);
    bool isModeIndexValid(u8 modeIndex);
    void setModeIndex(u8 modeIndex);
    bool isPackCourseCountValid(u8 packCourseCount);
    void setPackCourseCount(u8 packCourseCount);
    bool isPackHashElementValid(u32 i0, u8 packHashElement);
    void setPackHashElement(u32 i0, u8 packHashElement);
    bool isCourseIndexValid(u8 courseIndex);
    void setCourseIndex(u8 courseIndex);
    bool isClientsCountValid(u32 clientsCount);
    void setClientsCount(u32 clientsCount);
    ReplayClientReader *clientsElementReader(u32 i0);
    bool isTimeValid(u64 time);
    void setTime(u64 time);
    bool isRoomTypeValid(u8 roomType);
    void setRoomType(u8 roomType);
    bool isFormatValid(u8 format);
    void setFormat(u8 format);
    bool isRoomCodeValid(u64 roomCode);
    void setRoomCode(u64 roomCode);

    bool isPkElementValid(u32 i0, u8 pkElement);
    void setPkElement(u32 i0, u8 pkElement);
    bool isRegionValid(u8 region);
    void setRegion(u8 region);
    bool isPlatformCountValid(u32 platformCount);
    void setPlatformCount(u32 platformCount);
    bool isPlatformElementValid(u32 i0, u8 platformElement);
    void setPlatformElement(u32 i0, u8 platformElement);
    bool isPlayersCountValid(u32 playersCount);
    void setPlayersCount(u32 playersCount);
    ClientPlayerReader *playersElementReader(u32 i0);
    bool isTeamsCountValid(u32 teamsCount);
    void setTeamsCount(u32 teamsCount);
    bool isTeamsElementValid(u32 i0, u8 teamsElement);
    void setTeamsElement(u32 i0, u8 teamsElement);

    bool isProfileValid(u8 profile);
    void setProfile(u8 profile);
    bool isNameElementValid(u32 i0, u8 nameElement);
    void setNameElement(u32 i0, u8 nameElement);

    static void Init();
    static CubeReplayManager *Instance();

private:
    CubeReplayManager();

    OSThread &thread() override;
    void process() override;

    void addReplays(Array<char, 128> &path, Storage::NodeInfo &nodeInfo);
    void addReplay(const Array<char, 128> &path);

    static bool CompareReplaysByTime(const Replay &a, const Replay &b);

    Ring<Replay, MaxReplayCount> m_replays;
    Replay *m_replay;
    u32 m_clientIndex;
    u32 m_playerIndex;
    Array<u8, 4 * 1024> m_stack;
    OSThread m_thread;

    static CubeReplayManager *s_instance;
};
