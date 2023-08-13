#include <assert.h>

#include <dolphin/OSError.h>

void Assert(const char *file, s32 line, const char *expression) {
    OSPanic(file, line, expression);
}
