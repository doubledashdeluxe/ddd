#pragma once

extern "C" {
#include <dolphin/OSContext.h>
#include <dolphin/OSError.h>

#include <payload/Replace.h>
}

typedef void (*JUTExceptionHandler)(OSError osError, OSContext *osContext, u32 dsisr, u32 dar);

class JUTException {
public:
    static JUTExceptionHandler REPLACED(setPreUserCallback)(JUTExceptionHandler exceptionHandler);
    REPLACE static JUTExceptionHandler setPreUserCallback(JUTExceptionHandler exceptionHandler);

private:
    u8 _00[0xA4 - 0x00];
};
static_assert(sizeof(JUTException) == 0xA4);
