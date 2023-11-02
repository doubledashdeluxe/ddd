#pragma once

#include "common/Console.hh"

struct Context {
    uintptr_t mem2ArenaLo;
    uintptr_t mem2ArenaHi;
    Console *console;
};
