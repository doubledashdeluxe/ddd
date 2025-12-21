#pragma once

#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/Online.hh>

class ClientStateRace
    : public ClientState
    , private ServerStateReader
    , private ServerStateRaceReader
    , private ServerRaceStateReader
    , private ServerRaceStateMainReader
    , private ClientStateWriter::Race
    , private ClientStateRaceWriter {
public:
    ClientStateRace(const ClientPlatform &platform, Connection &connection,
            const ClientStateRaceWriteInfo &writeInfo);
    ~ClientStateRace() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStatePoll(const ClientStatePollWriteInfo &writeInfo) override;
    ClientState &writeStateRace(const ClientStateRaceWriteInfo &writeInfo) override;

private:
    typedef ClientStateRaceReadInfo ReadInfo;
    typedef ClientStateRaceWriteInfo WriteInfo;

    ServerStateServerReader *serverReader() override;
    ServerStateModeReader *modeReader() override;
    ServerStatePackReader *packReader() override;
    ServerStateRoomReader *roomReader() override;
    ServerStateTeamReader *teamReader() override;
    ServerStatePollReader *pollReader() override;
    ServerStateRaceReader *raceReader() override;

    ServerRaceStateReader *serverRaceStateReader() override;

    ServerRaceStateMainReader *mainReader() override;
    bool isErrorValid() override;
    void setError() override;

    bool isFrameValid(u16 frame) override;
    void setFrame(u16 frame) override;
    bool isClientFrameValid(u16 clientFrame) override;
    void setClientFrame(u16 clientFrame) override;

    ClientStateRaceWriter &raceWriter() override;

    u16 getFrame() override;

    ReadInfo m_readInfo;
    WriteInfo m_writeInfo;
};
