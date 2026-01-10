#pragma once

#include <jsystem/JKRArchive.hh>

class J2DManager {
public:
    JKRArchive *archive() const;
    s32 goalAnmFrame() const;
    void calc();

    static J2DManager *Instance();
    static u32 KartStatus(u32 kartIndex);
    static u32 StatusKart(u32 statusIndex);

private:
    JKRArchive *m_archive;
    u8 _04[0x0c - 0x04];
    s32 m_goalAnmFrame;
    u8 _10[0x48 - 0x10];

    static J2DManager *s_instance;
};
size_assert(J2DManager, 0x48);
