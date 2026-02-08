#pragma once

#include <jsystem/TVec3.hh>

class KartBody {
public:
    KartBody();

    u32 getGameStatus() const;
    u8 getDriver() const;
    u8 getIndex() const;

private:
    u8 _000[0x110 - 0x000];

public:
    Mtx34 m_mtx;
    Mtx34 m_bodyMtx;

private:
    u8 _170[0x23c - 0x170];

public:
    Vec3f m_pos;

private:
    u8 _248[0x260 - 0x248];

public:
    Vec3f m_vel;

private:
    u8 _26c[0x3ec - 0x26c];

public:
    f32 m_speed;

private:
    u8 _3f0[0x578 - 0x3f0];
    u32 m_gameStatus;
    u8 _57c[0x5b2 - 0x57c];
    u8 m_driver;
    u8 m_index;
    u8 _5b4[0x5c8 - 0x5b4];
};
size_assert(KartBody, 0x5c8);
