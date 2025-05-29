#pragma once

#include <portable/Array.hh>

class EXI {
public:
    class Device {
    public:
        Device(u32 channel, u32 device, u32 frequency, bool *wasDetached);
        ~Device();
        bool ok() const;
        bool immRead(void *buffer, u32 size);
        bool immWrite(const void *buffer, u32 size);

    private:
        u32 m_channel;
        bool m_ok;
    };

    static bool GetID(u32 channel, u32 device, u32 &id);

private:
    EXI();

    static void HandleUnlock(s32 channel, struct OSContext *context);

    static Array<struct OSThreadQueue, 3> s_queues;
};
