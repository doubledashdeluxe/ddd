#pragma once

#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

class ClientStateUpdate
    : public ClientState
    , public ClientState::Reader<ClientStateUpdate>
    , public ClientState::Writer<ClientStateUpdate>
    , public ServerStateReader<ClientStateUpdate>
    , public ServerStateUpdateReader<ClientStateUpdate>
    , public ServerUpdateStateReader<ClientStateUpdate>
    , public ServerUpdateStateInfoReader<ClientStateUpdate>
    , public ServerUpdateStateDataReader<ClientStateUpdate>
    , public ClientStateWriter<ClientStateUpdate>::Update
    , public ClientStateUpdateWriter<ClientStateUpdate>
    , public ClientUpdateStateWriter<ClientStateUpdate>::Info
    , public ClientUpdateStateInfoWriter<ClientStateUpdate>
    , public ClientUpdateStateWriter<ClientStateUpdate>::Data
    , public ClientUpdateStateDataWriter<ClientStateUpdate> {
public:
    ClientStateUpdate(const ClientPlatform &platform, Connection &connection,
            const Array<char, MaxVersionLength + 1> &version,
            const ClientStateUpdateWriteInfo &writeInfo);
    ~ClientStateUpdate() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateServer(const ClientStateServerWriteInfo &writeInfo) override;
    ClientState &writeStateUpdate(const ClientStateUpdateWriteInfo &writeInfo) override;

    ServerStateServerReader<void> *serverReader();
    ServerStateUpdateReader *updateReader();
    ServerStateModeReader<void> *modeReader();
    ServerStatePackReader<void> *packReader();
    ServerStateRoomReader<void> *roomReader();
    ServerStateTeamReader<void> *teamReader();
    ServerStatePollReader<void> *pollReader();
    ServerStateRaceReader<void> *raceReader();

    ServerUpdateStateReader *serverUpdateStateReader();

    ServerUpdateStateInfoReader *infoReader();
    ServerUpdateStateDataReader *dataReader();

    bool isSizeValid(u32 size);
    void setSize(u32 size);
    bool isChangelogCountValid(u32 changelogCount);
    void setChangelogCount(u32 changelogCount);
    bool isChangelogElementValid(u32 i0, u8 changelogElement);
    void setChangelogElement(u32 i0, u8 changelogElement);

    bool isIndexValid(u16 index);
    void setIndex(u16 index);
    bool isChunkElementValid(u32 i0, u8 chunkElement);
    void setChunkElement(u32 i0, u8 chunkElement);

    ClientStateUpdateWriter &updateWriter();

    ClientUpdateStateWriter &clientUpdateStateWriter();

    ClientUpdateStateInfoWriter &infoWriter();
    ClientUpdateStateDataWriter &dataWriter();

    u8 getRegion();
    u32 getPlatformCount();
    u8 getPlatformElement(u32 i0);
    u8 getLanguage();

    u32 getIndicesCount();
    u16 getIndicesElement(u32 i0);

private:
    typedef ClientStateUpdateReadInfo ReadInfo;
    typedef ClientStateUpdateWriteInfo WriteInfo;

    ReadInfo m_readInfo;
    WriteInfo m_writeInfo;
};
