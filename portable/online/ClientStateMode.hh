#pragma once

#include "portable/UniquePtr.hh"
#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/ClientState.hh>
#include <formats/ServerState.hh>

class ClientStateMode
    : public ClientState
    , private ServerStateReader
    , private ServerStateModeReader
    , private ServerModeReader
    , private ClientStateWriter::Mode
    , private ClientStateModeWriter {
public:
    ClientStateMode(const ClientPlatform &platform, Connection &connection, u8 playerCount);
    ~ClientStateMode() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateServer(const ClientStateServerWriteInfo &writeInfo) override;
    ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo) override;
    ClientState &writeStatePack(const ClientStatePackWriteInfo &writeInfo) override;

private:
    typedef ClientStateModeReadInfo ReadInfo;
    typedef ClientStateModeWriteInfo WriteInfo;

    ServerStateServerReader *serverReader() override;
    ServerStateModeReader *modeReader() override;
    ServerStatePackReader *packReader() override;

    bool isModesCountValid(u32 modesCount) override;
    void setModesCount(u32 modesCount) override;
    ServerModeReader *modesElementReader(u32 i0) override;

    bool isMmrsCountValid(u32 mmrsCount) override;
    void setMmrsCount(u32 mmrsCount) override;
    bool isMmrsElementValid(u32 i0, u16 mmrsElement) override;
    void setMmrsElement(u32 i0, u16 mmrsElement) override;
    bool isPlayerCountValid(u16 playerCount) override;
    void setPlayerCount(u16 playerCount) override;

    ClientStateModeWriter &modeWriter() override;

    void checkSocket();

    UniquePtr<Connection> m_connection;
    u8 m_playerCount;
    ReadInfo m_readInfo;
    u32 m_modeIndex;
};
