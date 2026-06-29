#pragma once

#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/Online.hh>

class ClientStatePack
    : public ClientState
    , public ClientState::Reader<ClientStatePack>
    , public ClientState::Writer<ClientStatePack>
    , public ServerStateReader<ClientStatePack>
    , public ServerStatePackReader<ClientStatePack>
    , public ClientStateWriter<ClientStatePack>::Pack
    , public ClientStatePackWriter<ClientStatePack> {
public:
    ClientStatePack(const ClientPlatform &platform, Connection &connection,
            const ClientStatePackWriteInfo &writeInfo);
    ~ClientStatePack() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo) override;
    ClientState &writeStatePack(const ClientStatePackWriteInfo &writeInfo) override;
    ClientState &writeStateRoom(const ClientStateRoomWriteInfo &writeInfo) override;

    ServerStateServerReader<void> *serverReader();
    ServerStateModeReader<void> *modeReader();
    ServerStatePackReader *packReader();
    ServerStateRoomReader<void> *roomReader();
    ServerStateTeamReader<void> *teamReader();
    ServerStatePollReader<void> *pollReader();
    ServerStateRaceReader<void> *raceReader();

    bool isModeIndexValid(u8 modeIndex);
    void setModeIndex(u8 modeIndex);
    bool isPackIndexValid(u8 packIndex);
    void setPackIndex(u8 packIndex);
    bool isPlayerCountValid(u16 playerCount);
    void setPlayerCount(u16 playerCount);
    bool isFormatPlayerCountsElementValid(u32 i0, u16 formatPlayerCountsElement);
    void setFormatPlayerCountsElement(u32 i0, u16 formatPlayerCountsElement);

    ClientStatePackWriter &packWriter();

    u8 getModeIndex();
    u8 getPackIndex();
    u8 getPackCourseCount();
    u8 getPackHashElement(u32 i0);

private:
    typedef ClientStatePackReadInfo ReadInfo;
    typedef ClientStatePackWriteInfo WriteInfo;

    ReadInfo m_readInfo;
    WriteInfo m_writeInfo;
    u8 m_packIndex;
};
