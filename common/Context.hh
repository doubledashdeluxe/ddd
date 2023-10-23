#pragma once

#include "common/Console.hh"

struct Context {
    u32 mem2ArenaLo;
    u32 mem2ArenaHi;
    Console *console;
};
