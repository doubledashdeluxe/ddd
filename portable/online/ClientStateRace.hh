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
    , private ServerRaceKartReader
    , private ClientStateWriter::Race
    , private ClientStateRaceWriter
    , private ClientRaceKartWriter {
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
    bool isKartFlagsValid(u8 kartFlags) override;
    void setKartFlags(u8 kartFlags) override;
    bool isKartsCountValid(u32 kartsCount) override;
    void setKartsCount(u32 kartsCount) override;
    ServerRaceKartReader *kartsElementReader(u32 i0) override;

    bool isKartFrameValid(u16 kartFrame) override;
    void setKartFrame(u16 kartFrame) override;
    bool isInputsCountValid(u32 inputsCount) override;
    void setInputsCount(u32 inputsCount) override;
    bool isInputsElementValid(u32 i0, u16 inputsElement) override;
    void setInputsElement(u32 i0, u16 inputsElement) override;
    bool isPosXValid(s16 posX) override;
    void setPosX(s16 posX) override;
    bool isPosYValid(s16 posY) override;
    void setPosY(s16 posY) override;
    bool isPosZValid(s16 posZ) override;
    void setPosZ(s16 posZ) override;
    bool isAngleValid(s8 angle) override;
    void setAngle(s8 angle) override;
    bool isVelXValid(s16 posX) override;
    void setVelX(s16 posX) override;
    bool isVelYValid(s16 posY) override;
    void setVelY(s16 posY) override;
    bool isVelZValid(s16 posZ) override;
    void setVelZ(s16 posZ) override;
    bool isItemFramesCountValid(u32 itemFramesCount) override;
    void setItemFramesCount(u32 itemFramesCount) override;
    bool isItemFramesElementValid(u32 i0, u16 itemFramesElement) override;
    void setItemFramesElement(u32 i0, u16 itemFramesElement) override;
    bool isItemIdsCountValid(u32 itemIdsCount) override;
    void setItemIdsCount(u32 itemIdsCount) override;
    bool isItemIdsElementValid(u32 i0, u8 itemIdsElement) override;
    void setItemIdsElement(u32 i0, u8 itemIdsElement) override;

    ClientStateRaceWriter &raceWriter() override;

    u16 getFrame() override;
    u32 getKartsCount() override;
    ClientRaceKartWriter &kartsElementWriter(u32 i0) override;
    u32 getItemCountsCount() override;
    u8 getItemCountsElement(u32 i0) override;

    u32 getInputsCount() override;
    u32 getInputsCount(u32 i0) override;
    u16 getInputsElement(u32 i0, u32 i1) override;
    s16 getPosX() override;
    s16 getPosY() override;
    s16 getPosZ() override;
    s8 getAngle() override;
    s16 getVelX() override;
    s16 getVelY() override;
    s16 getVelZ() override;
    u32 getItemFramesCount() override;
    u16 getItemFramesElement(u32 i0) override;
    u8 getRank() override;

    u32 m_kartIndex;
    ReadInfo m_readInfo;
    WriteInfo m_writeInfo;
};
