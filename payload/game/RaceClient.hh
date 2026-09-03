#pragma once

#include <jsystem/TVec3.hh>
#include <portable/online/ClientReadHandler.hh>
#include <portable/online/ClientStateRaceWriteInfo.hh>

class RaceClient : private ClientReadHandler {
public:
    struct ItemEvent {
        u8 frame;
        s8 stickY;
        u8 itemID;
        TVec3<f32> pos;
    };

    bool ok() const;
    u16 serverFrame() const;
    u16 clientFrame() const;
    u16 latency() const;
    u8 stability() const;
    s32 drift() const;
    void adjustDrift(s32 adjustment);
    u16 endFrame() const;
    bool hasResults() const;
    const Array<s32, 8> &pointDiffs() const;
    void setHasItem(u32 kartIndex, u32 characterIndex);
    bool hasItem(u32 kartIndex, u32 characterIndex) const;
    u32 itemID(u32 kartIndex, u32 characterIndex) const;
    void pushItemEvent(u32 kartIndex, const ItemEvent &itemEvent);
    const ItemEvent *itemEvent(u32 kartIndex) const;
    void popItemEvent(u32 kartIndex);
    void calcBefore();
    void calcAfter();
    void read();
    void write();

    static void Create();
    static void Destroy();
    static RaceClient *Instance();
    static u32 Frame();

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
        u8 driver;
        TVec3<f32> pos;
        f32 angle;
        Vec3f vel;
        Array<u16, KartCharacterCount> itemFrames;
        Array<u8, KartCharacterCount> itemIDs;
        u8 itemEventCounter;
        Ring<ItemEvent, MaxItemEventCount> itemEvents;
    };

    struct RankedKartIndexComparator {
        bool operator()(const u32 &a, const u32 &b);

        const Array<u32, 8> &points;
        const Array<u32, 8> &prevPoints;
    };

    RaceClient();
    virtual ~RaceClient();

    bool clientStatePoll(const ClientStatePollReadInfo &readInfo) override;
    bool clientStateRace(const ClientStateRaceReadInfo &readInfo) override;
    void clientStateError() override;

    template <typename T>
    static void UpdateItemEvents(Ring<T, MaxItemEventCount> &itemEvents) {
        for (u32 i = 0; i < itemEvents.count(); i++) {
            if (itemEvents[i].frame < MaxKartInputCount) {
                itemEvents[i].frame++;
            } else {
                itemEvents.popBack();
            }
        }
    }

    static f32 Convert(f32 value, f32 scale);
    static f32 Convert(s32 value, f32 scale);
    static s16 ConvertPos(f32 value);
    static f32 ConvertPos(s16 value);
    static s8 ConvertAngle(f32 value);
    static f32 ConvertAngle(s8 value);
    static s16 ConvertVel(f32 value);
    static f32 ConvertVel(s16 value);
    static f32 TruncateDiff(f32 diff, f32 scale);
    static f32 TruncatePosDiff(f32 diff);
    static f32 TruncateAngleDiff(f32 diff);
    static f32 TruncateVelDiff(f32 diff);

    bool m_ok;
    u16 m_serverFrame;
    u16 m_clientFrame;
    u16 m_latency;
    u8 m_stability;
    u64 m_stabilityFlags;
    s32 m_drift;
    Ring<s32, 60> m_drifts;
    u16 m_endFrame;
    bool m_hasResults;
    Array<s32, 8> m_pointDiffs;
    Array<Ring<KartState, 30>, MaxRoomKartCount> m_kartStates;
    Array<KartDiff, MaxRoomKartCount> m_kartDiffs;
    WriteInfo m_writeInfo;

    static RaceClient *s_instance;

    static const f32 PosScale;
    static const f32 AngleScale;
    static const f32 VelScale;
};
