#pragma once

extern "C" {
#include <dolphin/OSContext.h>
#include <dolphin/OSError.h>
}

#include <payload/Replace.hh>

typedef void (*JUTExceptionHandler)(OSError osError, OSContext *osContext, u32 dsisr, u32 dar);

class JUTException {
public:
    __attribute__((noreturn)) static void panic_f(const char *file, int line, const char *message,
            ...);
    static JUTExceptionHandler REPLACED(setPreUserCallback)(JUTExceptionHandler exceptionHandler);
    REPLACE static JUTExceptionHandler setPreUserCallback(JUTExceptionHandler exceptionHandler);

private:
    u8 _00[0xA4 - 0x00];
};
size_assert(JUTException, 0xA4);
