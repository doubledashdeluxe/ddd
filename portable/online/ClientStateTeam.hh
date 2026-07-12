#pragma once

#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/Online.hh>

class ClientStateTeam
    : public ClientState
    , public ClientState::Reader<ClientStateTeam>
    , public ClientState::Writer<ClientStateTeam>
    , public ServerStateReader<ClientStateTeam>
    , public ServerStateTeamReader<ClientStateTeam>
    , public ServerTeamStateReader<ClientStateTeam>
    , public ServerTeamStateMainReader<ClientStateTeam>
    , public ClientStateWriter<ClientStateTeam>::Team
    , public ClientStateTeamWriter<ClientStateTeam>
    , public ClientTeamStateWriter<ClientStateTeam>::Host
    , public ClientTeamStateHostWriter<ClientStateTeam>
    , public ClientTeamStateWriter<ClientStateTeam>::Guest
    , public ClientTeamStateGuestWriter<ClientStateTeam> {
public:
    ClientStateTeam(const ClientPlatform &platform, Connection &connection,
            const ClientStateTeamWriteInfo &writeInfo);
    ~ClientStateTeam() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateMode(const ClientStateModeWriteInfo &writeInfo) override;
    ClientState &writeStatePack(const ClientStatePackWriteInfo &writeInfo) override;
    ClientState &writeStateTeam(const ClientStateTeamWriteInfo &writeInfo) override;
    ClientState &writeStatePoll(const ClientStatePollWriteInfo &writeInfo) override;

    ServerStateServerReader<void> *serverReader();
    ServerStateUpdateReader<void> *updateReader();
    ServerStateModeReader<void> *modeReader();
    ServerStatePackReader<void> *packReader();
    ServerStateRoomReader<void> *roomReader();
    ServerStateTeamReader *teamReader();
    ServerStatePollReader<void> *pollReader();
    ServerStateRaceReader<void> *raceReader();

    ServerTeamStateReader *serverTeamStateReader();

    ServerTeamStateMainReader *mainReader();
    bool isErrorValid();
    void setError();

    bool isTeamsCountValid(u32 teamsCount);
    void setTeamsCount(u32 teamsCount);
    bool isTeamsElementValid(u32 i0, u8 teamsElement);
    void setTeamsElement(u32 i0, u8 teamsElement);
    bool isEntryIndexValid(u8 entryIndex);
    void setEntryIndex(u8 entryIndex);
    bool isContinuingValid(u8 continuing);
    void setContinuing(u8 continuing);

    ClientStateTeamWriter &teamWriter();

    ClientTeamStateWriter &clientTeamStateWriter();

    ClientTeamStateHostWriter &hostWriter();
    ClientTeamStateGuestWriter &guestWriter();

    u32 getTeamsCount();
    u8 getTeamsElement(u32 i0);
    u8 getEntryIndex();
    u8 getContinuing();

private:
    typedef ClientStateTeamReadInfo ReadInfo;
    typedef ClientStateTeamWriteInfo WriteInfo;

    ReadInfo m_readInfo;
    WriteInfo m_writeInfo;
};
