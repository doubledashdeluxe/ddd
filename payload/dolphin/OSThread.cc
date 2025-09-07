extern "C" {
#include "OSThread.h"
}

#include <cube/Panic.hh>
#include <payload/Lock.hh>
#include <payload/PerfOverlay.hh>

extern OSThreadQueue threadQueue;

static OSThread *thread;

extern "C" REPLACE void DefaultSwitchThreadCallback(OSThread *prev, OSThread *next) {
    PerfOverlay *perfOverlay = PerfOverlay::Instance();
    if (perfOverlay) {
        perfOverlay->handleSwitchThread(prev, next);
    }

    if (!prev) {
        return;
    }

    u32 *stackTop = prev->stackTop;
    u32 *sp = &prev->context.gprs[1];
    if (*sp <= reinterpret_cast<uintptr_t>(stackTop) || *stackTop != 0xdeadbabe) {
        if (prev != thread) {
            thread = prev;
            PANIC("Stack overflow detected!");
        }
    }
}

static bool IsThreadActive(OSThread *thread) {
    if (thread->state == OS_THREAD_STATE_EXITED) {
        return false;
    }
    for (OSThread *it = threadQueue.head; it; it = it->nextActive) {
        if (it == thread) {
            return true;
        }
    }
    return false;
}

extern "C" BOOL OSJoinThread(OSThread *thread, void **val) {
    Lock<NoInterrupts> lock;

    if (!(thread->flags & OS_THREAD_DETACHED) && thread->state != OS_THREAD_STATE_MORIBUND &&
            !thread->joinQueue.head) {
        OSSleepThread(&thread->joinQueue);
        if (!IsThreadActive(thread)) {
            return false;
        }
    }

    if (thread->state == OS_THREAD_STATE_MORIBUND) {
        if (val) {
            *val = thread->val;
        }

        if (thread->nextActive) {
            thread->nextActive->prevActive = thread->prevActive;
        } else {
            threadQueue.tail = thread->prevActive;
        }
        if (thread->prevActive) {
            thread->prevActive->nextActive = thread->nextActive;
        } else {
            threadQueue.head = thread->nextActive;
        }

        thread->state = OS_THREAD_STATE_EXITED;

        return true;
    }

    return false;
}
