#include <cube/Panic.h>

#include <portable/Log.h>

void Panic(const char *file, s32 line, const char *message) {
    ERROR("[%s:%d] %s", file, line, message);
    while (true) {}
}
