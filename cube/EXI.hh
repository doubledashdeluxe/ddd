#pragma once

#include <portable/Array.hh>

class EXI {
public:
    class Device {
    public:
        Device();
        Device(u32 channel, u32 device, u32 frequency, bool *wasDetached);
        ~Device();
        bool ok() const;
        bool acquire(u32 channel, u32 device, u32 frequency, bool *wasDetached);
        void release();
        bool immRead(void *buffer, u32 size);
        bool immWrite(const void *buffer, u32 size);
        bool dmaRead(void *buffer, u32 size);
        bool dmaWrite(const void *buffer, u32 size);

    private:
        u32 m_channel;
        bool m_ok;
    };

    static bool CanSwap(u32 channel, u32 device);
    static bool GetID(u32 channel, u32 device, u32 &id);

private:
    EXI();

    static void HandleUnlock(s32 channel, struct OSContext *context);

    static Array<struct OSThreadQueue, 3> s_queues;
};
