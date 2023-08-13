#pragma once

#include <common/Types.hh>

class SceneType {
public:
    enum {
        Title = 0x0,
        // ...
    };

private:
    SceneType();
};

class SequenceApp {
public:
    static void Call(s32 sceneType);

private:
    SequenceApp();

    u8 _00[0x4c - 0x00];
};
static_assert(sizeof(SequenceApp) == 0x4c);
