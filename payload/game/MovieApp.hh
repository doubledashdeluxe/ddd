#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

class MovieApp {
public:
    static MovieApp *REPLACED(Create)();
    REPLACE static MovieApp *Create();

private:
    MovieApp();

    u8 _00[0x2c - 0x00];
};
size_assert(MovieApp, 0x2c);
