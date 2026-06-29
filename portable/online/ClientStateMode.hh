#pragma once

#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/Online.hh>

class ClientStateMode
    : public ClientState
    , public ClientState::Reader<ClientStateMode>
    , public ClientState::Writer<ClientStateMode>
    , public ServerStateReader<ClientStateMode>
    , public ServerStateModeReader<ClientStateMode>
    , public ServerModeReader<ClientStateMode>
    , public ClientStateWriter<ClientStateMode>::Mode
    , public ClientStateModeWriter<ClientStateMode> {
public:
    ClientStateMode(const ClientPlatform &platform, Connection &connection, u8 playerCount);
    ~ClientStateMode() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateServer(const ClientStateServerWriteInfo &writeInfo) override;
    ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo) override;
    ClientState &writeStatePack(const ClientStatePackWriteInfo &writeInfo) override;
    ClientState &writeStateRoom(const ClientStateRoomWriteInfo &writeInfo) override;

    ServerStateServerReader<void> *serverReader();
    ServerStateModeReader *modeReader();
    ServerStatePackReader<void> *packReader();
    ServerStateRoomReader<void> *roomReader();
    ServerStateTeamReader<void> *teamReader();
    ServerStatePollReader<void> *pollReader();
    ServerStateRaceReader<void> *raceReader();

    ServerModeReader *modesElementReader(u32 i0);

    bool isMmrsCountValid(u32 mmrsCount);
    void setMmrsCount(u32 mmrsCount);
    bool isMmrsElementValid(u32 i0, u16 mmrsElement);
    void setMmrsElement(u32 i0, u16 mmrsElement);
    bool isPlayerCountValid(u16 playerCount);
    void setPlayerCount(u16 playerCount);

    ClientStateModeWriter &modeWriter();

private:
    typedef ClientStateModeReadInfo ReadInfo;
    typedef ClientStateModeWriteInfo WriteInfo;

    u8 m_playerCount;
    ReadInfo m_readInfo;
    u32 m_modeIndex;
};
