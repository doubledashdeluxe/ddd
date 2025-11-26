#pragma once

#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/Online.hh>

class ClientStateTeam
    : public ClientState
    , private ServerStateReader
    , private ServerStateTeamReader
    , private ServerTeamStateReader
    , private ServerTeamStateMainReader
    , private ClientStateWriter::Team
    , private ClientStateTeamWriter
    , private ClientTeamStateWriter::Host
    , private ClientTeamStateHostWriter
    , private ClientTeamStateWriter::Guest
    , private ClientTeamStateGuestWriter {
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

private:
    typedef ClientStateTeamReadInfo ReadInfo;
    typedef ClientStateTeamWriteInfo WriteInfo;

    ServerStateServerReader *serverReader() override;
    ServerStateModeReader *modeReader() override;
    ServerStatePackReader *packReader() override;
    ServerStateRoomReader *roomReader() override;
    ServerStateTeamReader *teamReader() override;
    ServerStatePollReader *pollReader() override;

    ServerTeamStateReader *serverTeamStateReader() override;

    ServerTeamStateMainReader *mainReader() override;
    bool isErrorValid() override;
    void setError() override;

    bool isTeamsCountValid(u32 teamsCount) override;
    void setTeamsCount(u32 teamsCount) override;
    bool isTeamsElementValid(u32 i0, u8 teamsElement) override;
    void setTeamsElement(u32 i0, u8 teamsElement) override;
    bool isEntryIndexValid(u8 entryIndex) override;
    void setEntryIndex(u8 entryIndex) override;
    bool isContinuingValid(u8 continuing) override;
    void setContinuing(u8 continuing) override;

    ClientStateTeamWriter &teamWriter() override;

    ClientTeamStateWriter &clientTeamStateWriter() override;

    ClientTeamStateHostWriter &hostWriter() override;
    ClientTeamStateGuestWriter &guestWriter() override;

    u32 getTeamsCount() override;
    u8 getTeamsElement(u32 i0) override;
    u8 getEntryIndex() override;
    u8 getContinuing() override;

    ReadInfo m_readInfo;
    WriteInfo m_writeInfo;
};
