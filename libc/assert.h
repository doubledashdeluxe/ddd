#pragma once

#include <common/Types.h>

void Assert(const char *file, s32 line, const char *expression);

#define assert(expression) \
    do { \
        if (!(expression)) { \
            Assert(__FILE__, __LINE__, #expression); \
        } \
    } while (false)
