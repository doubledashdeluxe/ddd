#pragma once

#include "portable/online/ClientState.hh"
#include "portable/online/Connection.hh"

#include <formats/Online.hh>

class ClientStateRace
    : public ClientState
    , public ClientState::Reader<ClientStateRace>
    , public ClientState::Writer<ClientStateRace>
    , public ServerStateReader<ClientStateRace>
    , public ServerStateRaceReader<ClientStateRace>
    , public ServerRaceStateReader<ClientStateRace>
    , public ServerRaceStateMainReader<ClientStateRace>
    , public ServerRaceKartReader<ClientStateRace>
    , public ItemEventReader<ClientStateRace>
    , public ServerResultReader<ClientStateRace>
    , public ClientStateWriter<ClientStateRace>::Race
    , public ClientStateRaceWriter<ClientStateRace>
    , public ClientRaceKartWriter<ClientStateRace>
    , public ItemEventWriter<ClientStateRace> {
public:
    ClientStateRace(const ClientPlatform &platform, Connection &connection,
            const ClientStateRaceWriteInfo &writeInfo);
    ~ClientStateRace() override;
    bool needsSockets() override;
    ClientState &read(ClientReadHandler &handler) override;
    ClientState &writeStateRoom(const ClientStateRoomWriteInfo &writeInfo) override;
    ClientState &writeStatePoll(const ClientStatePollWriteInfo &writeInfo) override;
    ClientState &writeStateRace(const ClientStateRaceWriteInfo &writeInfo) override;

    ServerStateServerReader<void> *serverReader();
    ServerStateUpdateReader<void> *updateReader();
    ServerStateModeReader<void> *modeReader();
    ServerStatePackReader<void> *packReader();
    ServerStateRoomReader<void> *roomReader();
    ServerStateTeamReader<void> *teamReader();
    ServerStatePollReader<void> *pollReader();
    ServerStateRaceReader *raceReader();

    ServerRaceStateReader *serverRaceStateReader();

    ServerRaceStateMainReader *mainReader();
    bool isErrorValid();
    void setError();

    bool isMatchIndexValid(u8 matchIndex);
    void setMatchIndex(u8 matchIndex);
    bool isFrameValid(u16 frame);
    void setFrame(u16 frame);
    bool isClientFrameValid(u16 clientFrame);
    void setClientFrame(u16 clientFrame);
    bool isKartFlagsValid(u8 kartFlags);
    void setKartFlags(u8 kartFlags);
    bool isKartsCountValid(u32 kartsCount);
    void setKartsCount(u32 kartsCount);
    ServerRaceKartReader *kartsElementReader(u32 i0);
    bool isEndFrameValid(u16 endFrame);
    void setEndFrame(u16 endFrame);
    bool isResultsCountValid(u32 resultsCount);
    void setResultsCount(u32 resultsCount);
    ServerResultReader *resultsElementReader(u32 i0);

    bool isKartFrameValid(u16 kartFrame);
    void setKartFrame(u16 kartFrame);
    bool isInputsCountValid(u32 inputsCount);
    void setInputsCount(u32 inputsCount);
    bool isInputsElementValid(u32 i0, u16 inputsElement);
    void setInputsElement(u32 i0, u16 inputsElement);
    bool isDriverValid(u8 driver);
    void setDriver(u8 driver);
    bool isPosXValid(s16 posX);
    void setPosX(s16 posX);
    bool isPosYValid(s16 posY);
    void setPosY(s16 posY);
    bool isPosZValid(s16 posZ);
    void setPosZ(s16 posZ);
    bool isAngleValid(s8 angle);
    void setAngle(s8 angle);
    bool isVelXValid(s16 posX);
    void setVelX(s16 posX);
    bool isVelZValid(s16 posZ);
    void setVelZ(s16 posZ);
    bool isItemFramesElementValid(u32 i0, u16 itemFramesElement);
    void setItemFramesElement(u32 i0, u16 itemFramesElement);
    bool isItemIdsElementValid(u32 i0, u8 itemIdsElement);
    void setItemIdsElement(u32 i0, u8 itemIdsElement);
    bool isItemEventCounterValid(u8 itemEventCounter);
    void setItemEventCounter(u8 itemEventCounter);
    bool isItemEventsCountValid(u32 itemEventsCount);
    void setItemEventsCount(u32 itemEventsCount);
    ItemEventReader *itemEventsElementReader(u32 i0);
    bool isLapValid(u8 lap);
    void setLap(u8 lap);
    bool isTimeValid(u32 time);
    void setTime(u32 time);

    bool isEventFrameValid(u8 itemFrame);
    void setEventFrame(u8 itemFrame);
    bool isEventStickYValid(s8 eventStickY);
    void setEventStickY(s8 eventStickY);
    bool isEventItemIdValid(u8 eventItemId);
    void setEventItemId(u8 eventItemId);
    bool isEventPosXValid(s16 eventPosX);
    void setEventPosX(s16 eventPosX);
    bool isEventPosZValid(s16 eventPosZ);
    void setEventPosZ(s16 eventPosZ);

    bool isKartIndexValid(u8 kartIndex);
    void setKartIndex(u8 kartIndex);
    bool isResultTimeValid(u32 resultTime);
    void setResultTime(u32 resultTime);
    bool isPointsValid(u16 points);
    void setPoints(u16 points);

    ClientStateRaceWriter &raceWriter();

    u16 getFrame();
    u32 getKartsCount();
    ClientRaceKartWriter &kartsElementWriter(u32 i0);
    u8 getItemCountsElement(u32 i0);
    u16 getLatency();
    u8 getStability();
    u32 getDelayedFrames();

    u32 getInputsCount();
    u32 getInputsCount(u32 i0);
    u16 getInputsElement(u32 i0, u32 i1);
    u8 getDriver();
    s16 getPosX();
    s16 getPosY();
    s16 getPosZ();
    s8 getAngle();
    s16 getVelX();
    s16 getVelZ();
    u16 getItemFramesElement(u32 i0);
    u8 getItemEventCounter();
    u32 getItemEventsCount();
    ItemEventWriter &itemEventsElementWriter(u32 i0);
    u8 getRank();
    u8 getLap();
    u32 getTime();

    u8 getEventFrame();
    s8 getEventStickY();
    u8 getEventItemId();
    s16 getEventPosX();
    s16 getEventPosZ();

private:
    typedef ClientStateRaceReadInfo ReadInfo;
    typedef ClientStateRaceWriteInfo WriteInfo;

    u32 m_kartIndex;
    u32 m_itemEventIndex;
    u32 m_resultIndex;
    ReadInfo m_readInfo;
    WriteInfo m_writeInfo;
};
