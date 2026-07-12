#pragma once

#include "portable/online/ClientState.hh"

#include <formats/Online.hh>

class ClientStateServer
    : public ClientState
    , public ClientState::Reader<ClientStateServer>
    , public ClientState::Writer<ClientStateServer>
    , public ServerStateReader<ClientStateServer>
    , public ServerStateServerReader<ClientStateServer>
    , public ServerIdentityReader<ClientStateServer>
    , public ServerIdentityUnspecifiedReader<ClientStateServer>
    , public ServerIdentitySpecifiedReader<ClientStateServer>
    , public ClientStateWriter<ClientStateServer>::Server
    , public ClientStateServerWriter<ClientStateServer>
    , public ClientIdentityWriter<ClientStateServer>::Unspecified
    , public ClientIdentityUnspecifiedWriter<ClientStateServer>
    , public ClientIdentityWriter<ClientStateServer>::Specified
    , public ClientIdentitySpecifiedWriter<ClientStateServer>
    , public ClientPlayerWriter<ClientStateServer> {
public:
    ClientStateServer(const ClientPlatform &platform);
    ~ClientStateServer() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateServer(const ClientStateServerWriteInfo &writeInfo) override;
    ClientState &writeStateUpdate(const ClientStateUpdateWriteInfo &writeInfo) override;
    ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo) override;

    ServerStateServerReader *serverReader();
    ServerStateUpdateReader<void> *updateReader();
    ServerStateModeReader<void> *modeReader();
    ServerStatePackReader<void> *packReader();
    ServerStateRoomReader<void> *roomReader();
    ServerStateTeamReader<void> *teamReader();
    ServerStatePollReader<void> *pollReader();
    ServerStateRaceReader<void> *raceReader();

    bool isUpdateVersionValid(u8 updateVersion);
    void setUpdateVersion(u8 updateVersion);
    bool isReservedValid(u8 reserved);
    void setReserved(u8 reserved);
    bool isProtocolVersionValid(u16 protocolVersion);
    void setProtocolVersion(u16 protocolVersion);
    bool isVersionCountValid(u32 versionCount);
    void setVersionCount(u32 versionCount);
    bool isVersionElementValid(u32 i0, u8 versionElement);
    void setVersionElement(u32 i0, u8 versionElement);
    ServerIdentityReader *serverIdentityReader();

    ServerIdentityUnspecifiedReader *unspecifiedReader();
    ServerIdentitySpecifiedReader *specifiedReader();

    bool isMotdCountValid(u32 motdCount);
    void setMotdCount(u32 motdCount);
    bool isMotdElementValid(u32 i0, u8 motdElement);
    void setMotdElement(u32 i0, u8 motdElement);
    bool isPlayerCountValid(u16 playerCount);
    void setPlayerCount(u16 playerCount);

    ClientStateServerWriter &serverWriter();

    u8 getUpdateVersion();
    u8 getReserved();
    u16 getProtocolVersion();
    u32 getVersionCount();
    u8 getVersionElement(u32 i0);
    ClientIdentityWriter &clientIdentityWriter();

    ClientIdentityUnspecifiedWriter &unspecifiedWriter();
    ClientIdentitySpecifiedWriter &specifiedWriter();

    u8 getFrameRate();
    u8 getRegion();
    u32 getPlatformCount();
    u8 getPlatformElement(u32 i0);
    u32 getPlayersCount();
    ClientPlayerWriter &playersElementWriter(u32 i0);
    u8 getKartCount();

    u8 getProfile();
    u8 getNameElement(u32 i0);

private:
    typedef ClientStateServerReadInfo ReadInfo;
    typedef ClientStateServerWriteInfo WriteInfo;

    void checkConnections();
    void checkServers();

    static bool UpdateIsAvailable(const ReadInfo::Server &server);

    ReadInfo m_readInfo;
    const WriteInfo *m_writeInfo;
    u32 m_playerIndex;
};
