#pragma once

#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/Online.hh>

class ClientStatePoll
    : public ClientState
    , private ServerStateReader
    , private ServerStatePollReader
    , private ServerPollStateReader
    , private ServerPollStatePendingReader
    , private ServerPollStateReadyReader
    , private ServerPollKartReader
    , private ClientStateWriter::Poll
    , private ClientStatePollWriter
    , private ClientPollStateWriter::Pending
    , private ClientPollStateWriter::Ready
    , private ClientPollStateReadyWriter
    , private ClientPollKartWriter
    , private ClientCourseIndexWriter::Unspecified
    , private ClientCourseIndexWriter::Specified {
public:
    ClientStatePoll(const ClientPlatform &platform, Connection &connection,
            const ClientStatePollWriteInfo &writeInfo);
    ~ClientStatePoll() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo) override;
    ClientState &writeStatePoll(const ClientStatePollWriteInfo &writeInfo) override;
    ClientState &writeStateRace(const ClientStateRaceWriteInfo &writeInfo) override;

private:
    typedef ClientStatePollReadInfo ReadInfo;
    typedef ClientStatePollWriteInfo WriteInfo;

    ServerStateServerReader *serverReader() override;
    ServerStateModeReader *modeReader() override;
    ServerStatePackReader *packReader() override;
    ServerStateRoomReader *roomReader() override;
    ServerStateTeamReader *teamReader() override;
    ServerStatePollReader *pollReader() override;
    ServerStateRaceReader *raceReader() override;

    ServerPollStateReader *serverPollStateReader() override;

    ServerPollStatePendingReader *pendingReader() override;
    ServerPollStateReadyReader *readyReader() override;
    bool isErrorValid() override;
    void setError() override;

    bool isKartIndicesCountValid(u32 kartIndicesCount) override;
    void setKartIndicesCount(u32 kartIndicesCount) override;
    bool isKartIndicesElementValid(u32 i0, u8 kartIndicesElement) override;
    void setKartIndicesElement(u32 i0, u8 kartIndicesElement) override;

    bool isKartsCountValid(u32 kartsCount) override;
    void setKartsCount(u32 kartsCount) override;
    ServerPollKartReader *kartsElementReader(u32 i0) override;
    bool isSelectedKartIndexValid(u8 selectedKartIndex) override;
    void setSelectedKartIndex(u8 selectedKartIndex) override;

    bool isKartIndexValid(u8 kartIndex) override;
    void setKartIndex(u8 kartIndex) override;
    bool isCharacterIdsCountValid(u32 characterIdsCount) override;
    void setCharacterIdsCount(u32 characterIdsCount) override;
    bool isCharacterIdsElementValid(u32 i0, u8 characterIdsElement) override;
    void setCharacterIdsElement(u32 i0, u8 characterIdsElement) override;
    bool isKartIdValid(u8 kartId) override;
    void setKartId(u8 kartId) override;
    bool isCourseIndexValid(u8 courseIndex) override;
    void setCourseIndex(u8 courseIndex) override;

    ClientStatePollWriter &pollWriter() override;

    ClientPollStateWriter &clientPollStateWriter() override;

    void getPending() override;
    ClientPollStateReadyWriter &readyWriter() override;

    u32 getKartsCount() override;
    ClientPollKartWriter &kartsElementWriter(u32 i0) override;
    ClientCourseIndexWriter &courseIndexWriter() override;

    u32 getCharacterIdsCount() override;
    u8 getCharacterIdsElement(u32 i0) override;
    u8 getKartId() override;

    void getUnspecified() override;
    u8 getSpecified() override;

    u32 m_kartIndex;
    ReadInfo m_readInfo;
    WriteInfo m_writeInfo;
};
