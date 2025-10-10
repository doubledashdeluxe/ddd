#pragma once

#include "portable/Ring.hh"
#include "portable/UniquePtr.hh"
#include "portable/online/ClientState.hh"
#include "portable/online/ClientStateServerInfo.hh"
#include "portable/online/Connection.hh"
#include "portable/online/ServerManager.hh"

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
    , private ClientIdentitySpecifiedWriter {
public:
    ClientStateServer(const ClientPlatform &platform);
    ~ClientStateServer() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateServer() override;

private:
    ServerStateServerReader *serverReader() override;
    ServerStateRoomReader *roomReader() override;

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
    u32 getPlayersCount(u32 i0) override;
    u32 getPlayersCount(u32 i0, u32 i1) override;
    u8 getPlayersElement(u32 i0, u32 i1, u32 i2) override;

    void checkConnections();
    void checkSocket();
    void checkServers();

    UniquePtr<Array<u8, 512>> m_buffer;
    Ring<UniquePtr<Connection>, ServerManager::MaxServerCount> m_connections;
    u32 m_readIndex;
    u32 m_writeIndex;
    ClientStateServerInfo m_info;
    Array<char, 20> m_version;
};
