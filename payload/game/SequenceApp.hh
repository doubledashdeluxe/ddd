#pragma once

#include "game/SceneType.hh"

#include <common/Types.hh>
#include <payload/Replace.hh>

class SequenceApp {
public:
    bool ready(s32 sceneType);
    void setNextScene(s32 sceneType);

    static void REPLACED(Call)(s32 sceneType);
    REPLACE static void Call(s32 sceneType);
    static SequenceApp *Instance();

private:
    SequenceApp();

    u8 _00[0x44 - 0x00];
    u32 m_state;
    u8 _48[0x4c - 0x48];

    static SequenceApp *s_instance;
    static u32 s_nextScene;
};
size_assert(SequenceApp, 0x4c);
