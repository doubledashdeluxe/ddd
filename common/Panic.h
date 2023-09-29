#pragma once

#include <common/Types.h>

__attribute__((noreturn)) void Panic(const char *file, s32 line, const char *message);

#define PANIC(message) Panic(__FILE__, __LINE__, message)
