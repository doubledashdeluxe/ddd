#pragma once

#include "portable/Ring.hh"
#include "portable/UniquePtr.hh"
#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"
#include "portable/online/ServerManager.hh"

#include <formats/ClientState.hh>
#include <formats/ServerState.hh>

class ClientStateServer
    : public ClientState
    , private ServerStateReader
    , private ServerStateServerReader
    , private ServerIdentityReader
    , private ServerIdentityUnspecifiedReader
    , private ServerIdentitySpecifiedReader
    , private ClientStateWriter::Server
    , private ClientStateServerWriter
    , private ClientIdentityWriter::Unspecified
    , private ClientIdentityUnspecifiedWriter
    , private ClientIdentityWriter::Specified
    , private ClientIdentitySpecifiedWriter
    , private ClientPlayerWriter {
public:
    ClientStateServer(const ClientPlatform &platform);
    ~ClientStateServer() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateServer(const ClientStateServerWriteInfo &writeInfo) override;
    ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo) override;

private:
    typedef ClientStateServerReadInfo ReadInfo;
    typedef ClientStateServerWriteInfo WriteInfo;

    ServerStateServerReader *serverReader() override;
    ServerStateModeReader *modeReader() override;
    ServerStatePackReader *packReader() override;

    bool isProtocolVersionValid(u32 protocolVersion) override;
    void setProtocolVersion(u32 protocolVersion) override;
    bool isVersionCountValid(u32 versionCount) override;
    void setVersionCount(u32 versionCount) override;
    bool isVersionElementValid(u32 i0, u8 versionElement) override;
    void setVersionElement(u32 i0, u8 versionElement) override;
    ServerIdentityReader *serverIdentityReader() override;

    ServerIdentityUnspecifiedReader *unspecifiedReader() override;
    ServerIdentitySpecifiedReader *specifiedReader() override;

    bool isMotdCountValid(u32 motdCount) override;
    void setMotdCount(u32 motdCount) override;
    bool isMotdElementValid(u32 i0, u8 motdElement) override;
    void setMotdElement(u32 i0, u8 motdElement) override;
    bool isPlayerCountValid(u16 playerCount) override;
    void setPlayerCount(u16 playerCount) override;

    ClientStateServerWriter &serverWriter() override;

    u32 getProtocolVersion() override;
    u32 getVersionCount() override;
    u8 getVersionElement(u32 i0) override;
    ClientIdentityWriter &clientIdentityWriter() override;

    ClientIdentityUnspecifiedWriter &unspecifiedWriter() override;
    ClientIdentitySpecifiedWriter &specifiedWriter() override;

    u32 getPlayersCount() override;
    ClientPlayerWriter &playersElementWriter(u32 i0) override;

    u8 getProfile() override;
    u32 getNameCount() override;
    u8 getNameElement(u32 i0) override;

    void checkConnections();
    void checkSocket();
    void checkServers();

    Ring<UniquePtr<Connection>, ServerManager::MaxServerCount> m_connections;
    u32 m_readIndex;
    u32 m_writeIndex;
    ReadInfo m_readInfo;
    Array<char, 20> m_version;
    const WriteInfo *m_writeInfo;
    u32 m_playerIndex;
};
