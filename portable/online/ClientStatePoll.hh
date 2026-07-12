#pragma once

#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/Online.hh>

class ClientStatePoll
    : public ClientState
    , public ClientState::Reader<ClientStatePoll>
    , public ClientState::Writer<ClientStatePoll>
    , public ServerStateReader<ClientStatePoll>
    , public ServerStatePollReader<ClientStatePoll>
    , public ServerPollStateReader<ClientStatePoll>
    , public ServerPollStatePendingReader<ClientStatePoll>
    , public ServerPollStateReadyReader<ClientStatePoll>
    , public ServerPollKartReader<ClientStatePoll>
    , public ClientStateWriter<ClientStatePoll>::Poll
    , public ClientStatePollWriter<ClientStatePoll>
    , public ClientPollStateWriter<ClientStatePoll>::Pending
    , public ClientPollStateWriter<ClientStatePoll>::Ready
    , public ClientPollStateReadyWriter<ClientStatePoll>
    , public ClientPollKartWriter<ClientStatePoll>
    , public ClientCourseIndexWriter<ClientStatePoll>::Unspecified
    , public ClientCourseIndexWriter<ClientStatePoll>::Specified {
public:
    ClientStatePoll(const ClientPlatform &platform, Connection &connection,
            const ClientStatePollWriteInfo &writeInfo);
    ~ClientStatePoll() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo) override;
    ClientState &writeStatePoll(const ClientStatePollWriteInfo &writeInfo) override;
    ClientState &writeStateRace(const ClientStateRaceWriteInfo &writeInfo) override;

    ServerStateServerReader<void> *serverReader();
    ServerStateUpdateReader<void> *updateReader();
    ServerStateModeReader<void> *modeReader();
    ServerStatePackReader<void> *packReader();
    ServerStateRoomReader<void> *roomReader();
    ServerStateTeamReader<void> *teamReader();
    ServerStatePollReader *pollReader();
    ServerStateRaceReader<void> *raceReader();

    ServerPollStateReader *serverPollStateReader();

    ServerPollStatePendingReader *pendingReader();
    ServerPollStateReadyReader *readyReader();
    bool isErrorValid();
    void setError();

    bool isKartIndicesCountValid(u32 kartIndicesCount);
    void setKartIndicesCount(u32 kartIndicesCount);
    bool isKartIndicesElementValid(u32 i0, u8 kartIndicesElement);
    void setKartIndicesElement(u32 i0, u8 kartIndicesElement);

    bool isKartsCountValid(u32 kartsCount);
    void setKartsCount(u32 kartsCount);
    ServerPollKartReader *kartsElementReader(u32 i0);
    bool isSelectedKartIndexValid(u8 selectedKartIndex);
    void setSelectedKartIndex(u8 selectedKartIndex);

    bool isKartIndexValid(u8 kartIndex);
    void setKartIndex(u8 kartIndex);
    bool isCharacterIdsElementValid(u32 i0, u8 characterIdsElement);
    void setCharacterIdsElement(u32 i0, u8 characterIdsElement);
    bool isKartIdValid(u8 kartId);
    void setKartId(u8 kartId);
    bool isCourseIndexValid(u8 courseIndex);
    void setCourseIndex(u8 courseIndex);

    ClientStatePollWriter &pollWriter();

    ClientPollStateWriter &clientPollStateWriter();

    void getPending();
    ClientPollStateReadyWriter &readyWriter();

    u32 getKartsCount();
    ClientPollKartWriter &kartsElementWriter(u32 i0);
    ClientCourseIndexWriter &courseIndexWriter();

    u8 getCharacterIdsElement(u32 i0);
    u8 getKartId();

    void getUnspecified();
    u8 getSpecified();

private:
    typedef ClientStatePollReadInfo ReadInfo;
    typedef ClientStatePollWriteInfo WriteInfo;

    u32 m_kartIndex;
    ReadInfo m_readInfo;
    WriteInfo m_writeInfo;
};
