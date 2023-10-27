#pragma once

#include <common/Types.hh>

class JASTaskThread {
public:
    typedef void Cmd(void *userData);

    bool sendCmdMsg(Cmd cmd, void *userData, size_t userDataSize);

private:
    u8 _00[0x88 - 0x00];
};
static_assert(sizeof(JASTaskThread) == 0x88);
