#pragma once

#include "game/GameApp.hh"

#include <payload/Replace.hh>

class MovieApp : public GameApp {
public:
    static MovieApp *REPLACED(Create)();
    REPLACE static MovieApp *Create();

private:
    MovieApp();

    u8 _00[0x2c - 0x0c];
};
size_assert(MovieApp, 0x2c);
