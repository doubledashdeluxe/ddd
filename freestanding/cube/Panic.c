#include <cube/Panic.h>

#include <cube/Log.h>

void Panic(const char *file, s32 line, const char *message) {
    ERROR("[%s:%d] %s", file, line, message);
    while (true) {}
}
