#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

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
    static void REPLACED(Call)(s32 sceneType);
    REPLACE static void Call(s32 sceneType);

private:
    SequenceApp();

    u8 _00[0x4c - 0x00];
};
size_assert(SequenceApp, 0x4c);
