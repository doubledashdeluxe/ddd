#pragma once

#include "game/KartGame.hh"

#include <jsystem/TVec3.hh>

class KartBody {
public:
    KartBody();

    KartGame *getGame() const;
    bool isChange() const;
    u32 getGameStatus() const;
    u8 getDriver() const;
    u8 getIndex() const;

private:
    u8 _000[0x0f0 - 0x000];
    KartGame *m_game;
    u8 _0f4[0x110 - 0x0f4];

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
    u8 _3f0[0x444 - 0x3f0];

public:
    f32 m_xzSpeed;
    f32 m_xyzSpeed;

private:
    u8 _44c[0x454 - 0x44c];

public:
    f32 m_bodySpeed;

private:
    u8 _458[0x470 - 0x458];

public:
    f32 m_speedScale;

private:
    u8 _474[0x574 - 0x474];
    u32 : 24;
    bool m_isChange : 1;
    u32 : 7;
    u32 m_gameStatus;
    u32 : 28;

public:
    bool m_itemTrigger : 1;
    bool m_itemBack : 1;
    bool m_itemFront : 1;
    bool m_itemThrow : 1;

private:
    u8 _580[0x5b2 - 0x580];
    u8 m_driver;
    u8 m_index;
    u8 _5b4[0x5c8 - 0x5b4];
};
size_assert(KartBody, 0x5c8);
