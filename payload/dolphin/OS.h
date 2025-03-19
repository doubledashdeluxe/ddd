#pragma once

#include "dolphin/OSContext.h"

typedef void (*OSExceptionHandler)(u8 exception, OSContext *context);

void OSInit(void);
OSExceptionHandler OSSetExceptionHandler(u8 exception, OSExceptionHandler handler);
