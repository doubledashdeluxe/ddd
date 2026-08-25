#pragma once

#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/Online.hh>

class ClientStateRoom
    : public ClientState
    , public ClientState::Reader<ClientStateRoom>
    , public ClientState::Writer<ClientStateRoom>
    , public ServerStateReader<ClientStateRoom>
    , public ServerStateRoomReader<ClientStateRoom>
    , public ServerRoomStateReader<ClientStateRoom>
    , public ServerRoomStateMainReader<ClientStateRoom>
    , public ServerKartReader<ClientStateRoom>
    , public ServerPlayerReader<ClientStateRoom>
    , public ServerRoomOptionsReader<ClientStateRoom>
    , public RoomOptionsRaceReader<ClientStateRoom>
    , public RoomOptionsBattleReader<ClientStateRoom>
    , public ClientStateWriter<ClientStateRoom>::Room
    , public ClientStateRoomWriter<ClientStateRoom>
    , public ClientRoomStateWriter<ClientStateRoom>::Search
    , public ClientRoomStateSearchWriter<ClientStateRoom>
    , public ClientRoomStateWriter<ClientStateRoom>::New
    , public ClientRoomStateNewWriter<ClientStateRoom>
    , public ClientRoomStateWriter<ClientStateRoom>::Code
    , public ClientRoomStateCodeWriter<ClientStateRoom>
    , public ClientRoomStateWriter<ClientStateRoom>::Main
    , public ClientRoomStateMainWriter<ClientStateRoom>
    , public ClientRoomOptionsWriter<ClientStateRoom>::RaceOptions
    , public ClientRoomOptionsWriter<ClientStateRoom>::BattleOptions
    , public ClientRoomOptionsWriter<ClientStateRoom>::None
    , public RoomOptionsRaceWriter<ClientStateRoom>
    , public RoomOptionsBattleWriter<ClientStateRoom> {
public:
    ClientStateRoom(const ClientPlatform &platform, Connection &connection,
            const ClientStateRoomWriteInfo &writeInfo);
    ~ClientStateRoom() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo) override;
    ClientState &writeStatePack(const ClientStatePackWriteInfo &writeInfo) override;
    ClientState &writeStateRoom(const ClientStateRoomWriteInfo &writeInfo) override;
    ClientState &writeStateTeam(const ClientStateTeamWriteInfo &writeInfo) override;
    ClientState &writeStatePoll(const ClientStatePollWriteInfo &writeInfo) override;

    ServerStateServerReader<void> *serverReader();
    ServerStateUpdateReader<void> *updateReader();
    ServerStateModeReader<void> *modeReader();
    ServerStatePackReader<void> *packReader();
    ServerStateRoomReader *roomReader();
    ServerStateTeamReader<void> *teamReader();
    ServerStatePollReader<void> *pollReader();
    ServerStateRaceReader<void> *raceReader();

    ServerRoomStateReader *serverRoomStateReader();

    ServerRoomStateMainReader *mainReader();
    bool isErrorValid();
    void setError();

    bool isKartsCountValid(u32 kartsCount);
    void setKartsCount(u32 kartsCount);
    ServerKartReader *kartsElementReader(u32 i0);
    bool isSpectatorCountValid(u16 spectatorCount);
    void setSpectatorCount(u16 spectatorCount);
    bool isModeIndexValid(u8 modeIndex);
    void setModeIndex(u8 modeIndex);
    bool isPackCourseCountValid(u8 packCourseCount);
    void setPackCourseCount(u8 packCourseCount);
    bool isPackHashElementValid(u32 i0, u8 packHashElement);
    void setPackHashElement(u32 i0, u8 packHashElement);
    bool isRoomCounterValid(u32 roomCounter);
    void setRoomCounter(u32 roomCounter);
    bool isRoomCodeValid(u64 roomCode);
    void setRoomCode(u64 roomCode);
    bool isSpectatingCounterValid(u32 spectatingCounter);
    void setSpectatingCounter(u32 spectatingCounter);
    bool isSpectatingValid(u8 spectating);
    void setSpectating(u8 spectating);
    ServerRoomOptionsReader *optionsReader();
    bool isContinuingValid(u8 continuing);
    void setContinuing(u8 continuing);

    bool isLocalValid(u8 local);
    void setLocal(u8 local);
    bool isPlayersCountValid(u32 playersCount);
    void setPlayersCount(u32 playersCount);
    ServerPlayerReader *playersElementReader(u32 i0);
    bool isMmrValid(u16 mmr);
    void setMmr(u16 mmr);
    bool isPointsValid(u16 points);
    void setPoints(u16 points);

    bool isIndexValid(u8 index);
    void setIndex(u8 index);
    bool isNameElementValid(u32 i0, u8 nameElement);
    void setNameElement(u32 i0, u8 nameElement);

    RoomOptionsRaceReader *raceOptionsReader();
    RoomOptionsBattleReader *battleOptionsReader();

    bool isRaceValid();
    void setRace();
    bool isBattleValid();
    void setBattle();
    bool isCodeTypeValid(u8 codeType);
    void setCodeType(u8 codeType);
    bool isFormatValid(u8 format);
    void setFormat(u8 format);
    bool isEngineSizeValid(u8 engineSize);
    void setEngineSize(u8 engineSize);
    bool isItemModeValid(u8 itemMode);
    void setItemMode(u8 itemMode);
    bool isLapCountValid(u8 lapCount);
    void setLapCount(u8 lapCount);
    bool isMatchCountValid(u8 matchCount);
    void setMatchCount(u8 matchCount);
    bool isCourseSelectionValid(u8 courseSelection);
    void setCourseSelection(u8 courseSelection);
    bool isEntryIndexValid(u8 entryIndex);
    void setEntryIndex(u8 entryIndex);

    ClientStateRoomWriter &roomWriter();

    ClientRoomStateWriter &clientRoomStateWriter();

    ClientRoomStateSearchWriter &searchWriter();
    ClientRoomStateNewWriter &newWriter();
    ClientRoomStateCodeWriter &codeWriter();
    ClientRoomStateMainWriter &mainWriter();

    u8 getModeIndex();
    u32 getPackCourseIndicesCount();
    u8 getPackCourseIndicesElement(u32 i0);
    u8 getFormat();
    u32 getRoomCounter();

    u64 getRoomCode();

    u32 getSpectatingCounter();
    u8 getSpectating();
    ClientRoomOptionsWriter &optionsWriter();
    u8 getContinuing();

    RoomOptionsRaceWriter &raceOptionsWriter();
    RoomOptionsBattleWriter &battleOptionsWriter();
    void getNone();

    void getRace();
    void getBattle();
    u8 getCodeType();
    u8 getEngineSize();
    u8 getItemMode();
    u8 getLapCount();
    u8 getMatchCount();
    u8 getCourseSelection();
    u8 getEntryIndex();

private:
    typedef ClientStateRoomReadInfo ReadInfo;
    typedef ClientStateRoomWriteInfo WriteInfo;

    ReadInfo m_readInfo;
    u32 m_kartIndex;
    u32 m_playerIndex;
    WriteInfo m_writeInfo;
};
