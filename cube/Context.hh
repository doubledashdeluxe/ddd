#pragma once

#include "cube/Console.hh"
#include "cube/VI.hh"

struct Context {
    bool hasVirtualDI;
    void *commonArchive;
    u32 commonArchiveSize;
    void *localizedArchive;
    u32 localizedArchiveSize;
    uintptr_t mem2ArenaLo;
    uintptr_t mem2ArenaHi;
    VI *vi;
    Console *console;
};
