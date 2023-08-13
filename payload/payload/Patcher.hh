#pragma once

#include <common/Types.hh>

class Patcher {
public:
    static void Run();

private:
    struct Patch {
        void *from;
        void *to;
        u32 *thunk;
    };

    Patcher();

    static const size_t PatchCount;
    static const Patch Patches[];
};
