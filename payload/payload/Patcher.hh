#pragma once

#include <portable/Types.hh>

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

    static void Wipe(u32 *insts, u32 instCount);

    static const size_t PatchCount;
    static const Patch Patches[];
};
