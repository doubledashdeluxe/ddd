#pragma once

#include <portable/online/ClientReadHandler.hh>
#include <portable/online/ClientStateRaceWriteInfo.hh>

class RaceClient : private ClientReadHandler {
public:
    bool ok() const;
    u16 clientFrame() const;
    Optional<s32> drift() const;
    void adjustDrift(s32 adjustment);
    void read();
    void write();

    static void Create();
    static void Destroy();
    static RaceClient *Instance();

private:
    RaceClient();
    virtual ~RaceClient();

    bool clientStatePoll(const ClientStatePollReadInfo &readInfo) override;
    bool clientStateRace(const ClientStateRaceReadInfo &readInfo) override;
    void clientStateError() override;

    bool m_ok;
    u16 m_frame;
    u16 m_clientFrame;
    s32 m_drift;
    Ring<s32, 60> m_drifts;
    ClientStateRaceWriteInfo m_writeInfo;

    static RaceClient *s_instance;
};
