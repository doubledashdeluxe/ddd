#pragma once

#include "game/SceneType.hh"

#include <common/Types.hh>
#include <payload/Replace.hh>

class SequenceApp {
public:
    static void REPLACED(Call)(s32 sceneType);
    REPLACE static void Call(s32 sceneType);

private:
    SequenceApp();

    u8 _00[0x4c - 0x00];
};
size_assert(SequenceApp, 0x4c);
