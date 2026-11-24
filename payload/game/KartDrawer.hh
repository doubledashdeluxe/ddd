#pragma once

#include "game/RaceKartLight.hh"

#include <payload/Replace.hh>
#include <portable/Types.hh>

class KartDrawer {
public:
    class Stage {
    public:
        enum {
            Ghost = 3,
        };

    private:
        Stage();
    };

    u8 ghostAlpha() const;
    bool isVisible() const;
    RaceKartLight *light(u32 index) const;
    void REPLACED(drawKart)(u32 screenIndex, u32 stage);
    REPLACE void drawKart(u32 screenIndex, u32 stage);
    void REPLACED(drawDriver)(u32 screenIndex, u32 stage);
    REPLACE void drawDriver(u32 screenIndex, u32 stage);

private:
    u32 convertStage(u32 screenIndex, u32 stage);

    u32 m_index;
    u8 _004[0x010 - 0x004];
    u32 m_stage;
    u8 m_ghostAlpha;
    u8 _015[0x018 - 0x015];
    u16 m_hiddenFlags;
    u8 _01a[0x030 - 0x01a];
    RaceKartLight *m_lights[4];
    u8 _040[0x178 - 0x040];
};
size_assert(KartDrawer, 0x178);
