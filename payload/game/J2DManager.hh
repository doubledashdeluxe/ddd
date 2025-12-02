#pragma once

#include <portable/Types.hh>

class J2DManager {
public:
    s32 goalAnmFrame() const;

    static J2DManager *Instance();
    static u32 KartStatus(u32 kartIndex);

private:
    u8 _00[0x0c - 0x00];
    s32 m_goalAnmFrame;
    u8 _10[0x48 - 0x10];

    static J2DManager *s_instance;
};
size_assert(J2DManager, 0x48);
