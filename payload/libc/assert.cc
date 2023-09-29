extern "C" {
#include <assert.h>
}

#include <jsystem/JUTException.hh>

extern "C" void Assert(const char *file, s32 line, const char *expression) {
    JUTException::panic_f(file, line, "%s", expression);
}
