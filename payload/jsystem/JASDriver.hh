#pragma once

#include <portable/Types.hh>

class JASDriver {
public:
    typedef s32 (*Callback)(void *userData);

    static bool RegisterSubFrameCallback(Callback callback, void *userData);
    static bool RejectCallback(Callback callback, void *userData);

private:
    JASDriver();
};
