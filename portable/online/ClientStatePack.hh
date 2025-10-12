#pragma once

#include "portable/UniquePtr.hh"
#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/ClientState.hh>
#include <formats/ServerState.hh>

class ClientStatePack
    : public ClientState
    , private ServerStateReader
    , private ServerStatePackReader
    , private ClientStateWriter::Pack
    , private ClientStatePackWriter {
public:
    ClientStatePack(const ClientPlatform &platform, Connection &connection,
            const ClientStatePackWriteInfo &writeInfo);
    ~ClientStatePack() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo) override;
    ClientState &writeStatePack(const ClientStatePackWriteInfo &writeInfo) override;

private:
    typedef ClientStatePackReadInfo ReadInfo;
    typedef ClientStatePackWriteInfo WriteInfo;

    ServerStateServerReader *serverReader() override;
    ServerStateModeReader *modeReader() override;
    ServerStatePackReader *packReader() override;

    bool isModeIndexValid(u8 modeIndex) override;
    void setModeIndex(u8 modeIndex) override;
    bool isPackIndexValid(u8 packIndex) override;
    void setPackIndex(u8 packIndex) override;
    bool isPlayerCountValid(u16 playerCount) override;
    void setPlayerCount(u16 playerCount) override;

    ClientStatePackWriter &packWriter() override;

    u8 getModeIndex() override;
    u8 getPackIndex() override;
    u32 getPackHashCount() override;
    u8 getPackHashElement(u32 i0) override;

    void checkSocket();

    UniquePtr<Connection> m_connection;
    ReadInfo m_readInfo;
    WriteInfo m_writeInfo;
    u8 m_packIndex;
};
