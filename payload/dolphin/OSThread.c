#include "OSThread.h"

#include <common/Panic.h>
#include <payload/Replace.h>

static OSThread *thread = NULL;

REPLACE void DefaultSwitchThreadCallback(OSThread *prev, OSThread * /* next */) {
    if (!prev) {
        return;
    }

    u32 *stackTop = prev->stackTop;
    u32 *sp = &prev->context.gprs[1];
    if (*sp <= (uintptr_t)stackTop || *stackTop != 0xdeadbabe) {
        if (prev != thread) {
            thread = prev;
            PANIC("Stack overflow detected!");
        }
    }
}
