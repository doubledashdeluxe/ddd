#pragma once

#include "common/Console.hh"

struct Context {
    void *commonArchive;
    u32 commonArchiveSize;
    uintptr_t mem2ArenaLo;
    uintptr_t mem2ArenaHi;
    Console *console;
};
