#include <cube/Panic.hh>

#include <jsystem/JUTException.hh>

extern "C" void Panic(const char *file, s32 line, const char *message) {
    JUTException::panic_f(file, line, "%s", message);
}
