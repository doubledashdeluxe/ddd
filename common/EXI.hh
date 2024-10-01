#pragma once

#include "common/Types.hh"

class EXI {
public:
    class Device {
    public:
        Device(u32 channel, u32 device, u32 frequency);
        ~Device();
        bool ok() const;
        bool immRead(void *buffer, u32 size);
        bool immWrite(const void *buffer, u32 size);

    private:
        u32 m_channel;
        bool m_ok;
    };

private:
    EXI();
};
