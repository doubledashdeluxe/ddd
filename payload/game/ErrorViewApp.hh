#pragma once

#include "game/GameApp.hh"

class ErrorViewApp : public GameApp {
public:
    static void Call(s32 errorID);

private:
    u8 _0c[0x1c - 0x0c];
};
size_assert(ErrorViewApp, 0x1c);
