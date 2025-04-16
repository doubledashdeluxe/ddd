#pragma once

#include <portable/Array.hh>

class SystemRecord {
public:
    class SecretKart {
    public:
        enum {
            None = 0 << 0,
            BabyLuigi = 1 << 0, // Rattle Buggy
            Patapata = 1 << 1,  // Para Wing
            Diddy = 1 << 2,     // Barrel Train
            KoopaJr = 1 << 3,   // Bullet Blaster
            Kinopio = 1 << 4,   // Toad Kart
            Kinopico = 1 << 5,  // Toadette Kart
            Luigi = 1 << 6,     // Green Fire
            Daisy = 1 << 7,     // Bloom Coach
            Catherine = 1 << 8, // Turbo Birdo
            Waluigi = 1 << 9,   // Waluigi Racer
            Pakkun = 1 << 10,   // Piranha Pipes
            Teressa = 1 << 11,  // Boo Pipes
            Extra = 1 << 12,    // Parade Kart
            All = (1 << 13) - 1,
        };

    private:
        SecretKart();
    };

    class GameFlag {
    public:
        enum {
            None = 0 << 0,
            KinopioCombi = 1 << 0,  // Toad and Toadette
            BossCombi = 1 << 1,     // Petey Piranha and King Boo
            Mirror = 1 << 2,        // Mirror Mode
            SpecialCup = 1 << 3,    // Special Cup
            AllCupTour = 1 << 4,    // All Cup Tour
            Ending = 1 << 5,        // Ending
            SpecialEnding = 1 << 6, // Special Ending
            MiniLuigi = 1 << 7,     // Luigi's Mansion
            MiniMario = 1 << 8,     // Tilt-A-Kart
            All = (1 << 9) - 1,
        };

    private:
        GameFlag();
    };

    void applyAspectRatioSetting();

    void setSecretKart(u16 secretKart);
    void setGameFlag(u16 gameFlag);
    void unlockAll();

    static SystemRecord &Instance();

public:
    u8 m_rearViewButtons : 8; // Added
    u8 m_aspectRatio : 4;     // Added

private:
    u32 : 16;

public:
    bool m_noTAGhosts : 1;
    bool m_noRumble : 1;
    u8 m_soundMode : 2;

private:
    u16 m_secretKart;
    u16 m_gameFlag;

public:
    s8 m_volume;
    u8 m_itemSlotType;
    u8 m_vsLapNum;
    Array<char, 4> m_name;

private:
    u8 _00f[0x614 - 0x00f];

    static SystemRecord s_instance;
};
size_assert(SystemRecord, 0x614);
