#pragma once

#include "payload/network/UDPSocket.hh"
#include "payload/online/ClientState.hh"
#include "payload/online/Connection.hh"
#include "payload/online/ServerManager.hh"

#include <common/Ring.hh>
#include <common/UniquePtr.hh>

class ClientStateServer
    : public ClientState
    , private ServerStateReader
    , private ServerStateServerReader
    , private ServerVersionReader
    , private ServerIdentityReader
    , private ServerIdentityUnspecifiedReader
    , private ServerIdentitySpecifiedReader
    , private ClientStateWriter::Server
    , private ClientStateServerWriter
    , private ClientVersionWriter
    , private ClientIdentityWriter::Unspecified
    , private ClientIdentityUnspecifiedWriter {
public:
    ClientStateServer(JKRHeap *heap, UDPSocket *socket, Array<u8, 512> *buffer);
    ~ClientStateServer() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateServer() override;

private:
    ServerStateServerReader *serverReader() override;
    ServerStateRoomReader *roomReader() override;

    bool isProtocolVersionValid(u32 protocolVersion) override;
    void setProtocolVersion(u32 protocolVersion) override;
    ServerVersionReader *serverVersionReader() override;
    ServerIdentityReader *serverIdentityReader() override;

    bool isMajorValid(u8 major) override;
    void setMajor(u8 major) override;
    bool isMinorValid(u8 minor) override;
    void setMinor(u8 minor) override;
    bool isPatchValid(u8 patch) override;
    void setPatch(u8 patch) override;

    ServerIdentityUnspecifiedReader *unspecifiedReader() override;
    ServerIdentitySpecifiedReader *specifiedReader() override;

    ClientStateServerWriter &serverWriter() override;

    u32 getProtocolVersion() override;
    ClientVersionWriter &clientVersionWriter() override;
    ClientIdentityWriter &clientIdentityWriter() override;

    u8 getMajor() override;
    u8 getMinor() override;
    u8 getPatch() override;

    ClientIdentityUnspecifiedWriter &unspecifiedWriter() override;

    void checkConnections();
    void checkSocket();

    UniquePtr<UDPSocket> m_socket;
    UniquePtr<Array<u8, 512>> m_buffer;
    Ring<UniquePtr<Connection>, ServerManager::MaxServerCount> m_connections;
    u32 m_index;
};
