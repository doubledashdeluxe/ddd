#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

class MovieApp {
public:
    REPLACE static MovieApp *Create();

private:
    MovieApp();

    u8 _00[0x2c - 0x00];
};
static_assert(sizeof(MovieApp) == 0x2c);
