#pragma once

#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/Online.hh>

class ClientStateRoom
    : public ClientState
    , private ServerStateReader
    , private ServerStateRoomReader
    , private ServerRoomStateReader
    , private ServerRoomStateMainReader
    , private ServerKartReader
    , private ServerPlayerReader
    , private ServerRoomOptionsReader
    , private RoomOptionsRaceReader
    , private RoomOptionsBattleReader
    , private ClientStateWriter::Room
    , private ClientStateRoomWriter
    , private ClientRoomStateWriter::New
    , private ClientRoomStateNewWriter
    , private ClientRoomStateWriter::Code
    , private ClientRoomStateCodeWriter
    , private ClientRoomStateWriter::Main
    , private ClientRoomStateMainWriter
    , private ClientRoomOptionsWriter::Race
    , private ClientRoomOptionsWriter::Battle
    , private ClientRoomOptionsWriter::None
    , private RoomOptionsRaceWriter
    , private RoomOptionsBattleWriter {
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

private:
    typedef ClientStateRoomReadInfo ReadInfo;
    typedef ClientStateRoomWriteInfo WriteInfo;

    ServerStateServerReader *serverReader() override;
    ServerStateModeReader *modeReader() override;
    ServerStatePackReader *packReader() override;
    ServerStateRoomReader *roomReader() override;
    ServerStateTeamReader *teamReader() override;
    ServerStatePollReader *pollReader() override;

    ServerRoomStateReader *serverRoomStateReader() override;

    ServerRoomStateMainReader *mainReader() override;
    bool isErrorValid() override;
    void setError() override;

    bool isKartsCountValid(u32 kartsCount) override;
    void setKartsCount(u32 kartsCount) override;
    ServerKartReader *kartsElementReader(u32 i0) override;
    bool isSpectatorCountValid(u16 spectatorCount) override;
    void setSpectatorCount(u16 spectatorCount) override;
    bool isModeIndexValid(u8 modeIndex) override;
    void setModeIndex(u8 modeIndex) override;
    bool isPackCourseCountValid(u8 packCourseCount) override;
    void setPackCourseCount(u8 packCourseCount) override;
    bool isPackHashCountValid(u32 packHashCount) override;
    void setPackHashCount(u32 packHashCount) override;
    bool isPackHashElementValid(u32 i0, u8 packHashElement) override;
    void setPackHashElement(u32 i0, u8 packHashElement) override;
    bool isRoomCounterValid(u32 roomCounter) override;
    void setRoomCounter(u32 roomCounter) override;
    bool isRoomCodeValid(u64 roomCode) override;
    void setRoomCode(u64 roomCode) override;
    bool isSpectatingCounterValid(u32 spectatingCounter) override;
    void setSpectatingCounter(u32 spectatingCounter) override;
    bool isSpectatingValid(u8 spectating) override;
    void setSpectating(u8 spectating) override;
    ServerRoomOptionsReader *optionsReader() override;
    bool isContinuingValid(u8 continuing) override;
    void setContinuing(u8 continuing) override;

    bool isLocalValid(u8 local) override;
    void setLocal(u8 local) override;
    bool isPlayersCountValid(u32 playersCount) override;
    void setPlayersCount(u32 playersCount) override;
    ServerPlayerReader *playersElementReader(u32 i0) override;
    bool isMmrValid(u16 mmr) override;
    void setMmr(u16 mmr) override;

    bool isIndexValid(u8 index) override;
    void setIndex(u8 index) override;
    bool isNameCountValid(u32 nameCount) override;
    void setNameCount(u32 nameCount) override;
    bool isNameElementValid(u32 i0, u8 nameElement) override;
    void setNameElement(u32 i0, u8 nameElement) override;

    RoomOptionsRaceReader *raceReader() override;
    RoomOptionsBattleReader *battleReader() override;

    bool isRaceValid() override;
    void setRace() override;
    bool isBattleValid() override;
    void setBattle() override;
    bool isCodeTypeValid(u8 codeType) override;
    void setCodeType(u8 codeType) override;
    bool isFormatValid(u8 format) override;
    void setFormat(u8 format) override;
    bool isEngineSizeValid(u8 engineSize) override;
    void setEngineSize(u8 engineSize) override;
    bool isItemModeValid(u8 itemMode) override;
    void setItemMode(u8 itemMode) override;
    bool isLapCountValid(u8 lapCount) override;
    void setLapCount(u8 lapCount) override;
    bool isMatchCountValid(u8 matchCount) override;
    void setMatchCount(u8 matchCount) override;
    bool isCourseSelectionValid(u8 courseSelection) override;
    void setCourseSelection(u8 courseSelection) override;
    bool isEntryIndexValid(u8 entryIndex) override;
    void setEntryIndex(u8 entryIndex) override;

    ClientStateRoomWriter &roomWriter() override;

    ClientRoomStateWriter &clientRoomStateWriter() override;

    ClientRoomStateNewWriter &newWriter() override;
    ClientRoomStateCodeWriter &codeWriter() override;
    ClientRoomStateMainWriter &mainWriter() override;

    u8 getModeIndex() override;
    u8 getPackCourseCount() override;
    u32 getPackHashCount() override;
    u8 getPackHashElement(u32 i0) override;
    u32 getRoomCounter() override;

    u64 getRoomCode() override;

    u32 getSpectatingCounter() override;
    u8 getSpectating() override;
    ClientRoomOptionsWriter &optionsWriter() override;
    u8 getContinuing() override;

    RoomOptionsRaceWriter &raceWriter() override;
    RoomOptionsBattleWriter &battleWriter() override;
    void getNone() override;

    void getRace() override;
    void getBattle() override;
    u8 getCodeType() override;
    u8 getFormat() override;
    u8 getEngineSize() override;
    u8 getItemMode() override;
    u8 getLapCount() override;
    u8 getMatchCount() override;
    u8 getCourseSelection() override;
    u8 getEntryIndex() override;

    ReadInfo m_readInfo;
    u32 m_kartIndex;
    u32 m_playerIndex;
    WriteInfo m_writeInfo;
};
