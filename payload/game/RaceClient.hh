#pragma once

#include <jsystem/TVec3.hh>
#include <portable/online/ClientReadHandler.hh>
#include <portable/online/ClientStateRaceWriteInfo.hh>

class RaceClient : private ClientReadHandler {
public:
    bool ok() const;
    u16 clientFrame() const;
    s32 drift() const;
    void adjustDrift(s32 adjustment);
    void updateInputs();
    void read();
    void write();

    static void Create();
    static void Destroy();
    static RaceClient *Instance();

private:
    typedef ClientStateRaceReadInfo ReadInfo;
    typedef ClientStateRaceWriteInfo WriteInfo;

    struct KartState {
        u16 frame;
        Vec3f pos;
        f32 angle;
        Vec3f vel;
    };

    struct KartDiff {
        Array<u16, MaxKartPlayerCount> inputs;
        TVec3<f32> pos;
        f32 angle;
        Vec3f vel;
    };

    RaceClient();
    virtual ~RaceClient();

    bool clientStatePoll(const ClientStatePollReadInfo &readInfo) override;
    bool clientStateRace(const ClientStateRaceReadInfo &readInfo) override;
    void clientStateError() override;

    static f32 Convert(s32 value, f32 scale);
    static f32 TruncateDiff(f32 diff, f32 scale);

    bool m_ok;
    u16 m_frame;
    u16 m_clientFrame;
    s32 m_drift;
    Ring<s32, 60> m_drifts;
    Array<Ring<KartState, 30>, MaxRoomKartCount> m_kartStates;
    Array<KartDiff, MaxRoomKartCount> m_kartDiffs;
    WriteInfo m_writeInfo;

    static RaceClient *s_instance;
};
