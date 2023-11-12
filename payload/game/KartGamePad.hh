#pragma once

#include <jsystem/JUTGamePad.hh>

class KartGamePad : public JUTGamePad {
public:
    KartGamePad(s32 basePadPort, s32 padPort, s32 padType, s32 padState);
    ~KartGamePad() override;

    static KartGamePad *GamePad(u32 index);
    static KartGamePad *KartPad(u32 index);

private:
    static KartGamePad *s_gamePads[4];
    static KartGamePad *s_kartPads[16];
};
size_assert(KartGamePad, 0xb8);
