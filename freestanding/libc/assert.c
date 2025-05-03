#include <assert.h>

#include <portable/Log.h>

void Assert(const char *file, s32 line, const char *expression) {
    ERROR("[%s:%d] %s", file, line, expression);
    while (true) {}
}
