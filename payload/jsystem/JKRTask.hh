#pragma once

#include <common/Types.hh>

class JKRTask {
public:
    typedef void (*Task)(void *userData);

    bool request(Task task, void *userData, void *r6);

private:
    u8 _00[0x98 - 0x00];
};
size_assert(JKRTask, 0x98);
