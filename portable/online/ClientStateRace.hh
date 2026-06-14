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
    , private ItemEventReader
    , private ClientStateWriter::Race
    , private ClientStateRaceWriter
    , private ClientRaceKartWriter
    , private ItemEventWriter {
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
    bool isDriverValid(u8 driver) override;
    void setDriver(u8 driver) override;
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
    bool isVelZValid(s16 posZ) override;
    void setVelZ(s16 posZ) override;
    bool isItemFramesElementValid(u32 i0, u16 itemFramesElement) override;
    void setItemFramesElement(u32 i0, u16 itemFramesElement) override;
    bool isItemIdsElementValid(u32 i0, u8 itemIdsElement) override;
    void setItemIdsElement(u32 i0, u8 itemIdsElement) override;
    bool isItemEventCounterValid(u8 itemEventCounter) override;
    void setItemEventCounter(u8 itemEventCounter) override;
    bool isItemEventsCountValid(u32 itemEventsCount) override;
    void setItemEventsCount(u32 itemEventsCount) override;
    ItemEventReader *itemEventsElementReader(u32 i0) override;

    bool isEventFrameValid(u8 itemFrame) override;
    void setEventFrame(u8 itemFrame) override;
    bool isEventStickYValid(s8 eventStickY) override;
    void setEventStickY(s8 eventStickY) override;
    bool isEventItemIdValid(u8 eventItemId) override;
    void setEventItemId(u8 eventItemId) override;
    bool isEventPosXValid(s16 eventPosX) override;
    void setEventPosX(s16 eventPosX) override;
    bool isEventPosZValid(s16 eventPosZ) override;
    void setEventPosZ(s16 eventPosZ) override;

    ClientStateRaceWriter &raceWriter() override;

    u16 getFrame() override;
    u32 getKartsCount() override;
    ClientRaceKartWriter &kartsElementWriter(u32 i0) override;
    u8 getItemCountsElement(u32 i0) override;

    u32 getInputsCount() override;
    u32 getInputsCount(u32 i0) override;
    u16 getInputsElement(u32 i0, u32 i1) override;
    u8 getDriver() override;
    s16 getPosX() override;
    s16 getPosY() override;
    s16 getPosZ() override;
    s8 getAngle() override;
    s16 getVelX() override;
    s16 getVelZ() override;
    u16 getItemFramesElement(u32 i0) override;
    u8 getItemEventCounter() override;
    u32 getItemEventsCount() override;
    ItemEventWriter &itemEventsElementWriter(u32 i0) override;
    u8 getRank() override;

    u8 getEventFrame() override;
    s8 getEventStickY() override;
    u8 getEventItemId() override;
    s16 getEventPosX() override;
    s16 getEventPosZ() override;

    u32 m_kartIndex;
    u32 m_itemEventIndex;
    ReadInfo m_readInfo;
    WriteInfo m_writeInfo;
};
