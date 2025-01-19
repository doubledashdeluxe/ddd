extern "C" {
#include "DVD.h"
}

#include <common/Clock.hh>
#include <common/VirtualDI.hh>
#include <payload/Lock.hh>
#include <payload/Mutex.hh>
#include <payload/Replace.hh>

#ifdef __CWCC__
static Mutex s_mutex;
#endif

extern "C" BOOL REPLACED(DVDReadAbsAsyncPrio)(DVDCommandBlock *block, void *addr, s32 length,
        s32 offset, DVDCBCallback callback, s32 prio);
extern "C" REPLACE BOOL DVDReadAbsAsyncPrio(DVDCommandBlock *block, void *addr, s32 length,
        s32 offset, DVDCBCallback callback, s32 prio) {
    if (!VirtualDI::IsInit()) {
        return REPLACED(DVDReadAbsAsyncPrio)(block, addr, length, offset, callback, prio);
    }

#ifdef __CWCC__
    Lock<Mutex> lock(s_mutex);
#endif

    block->command = 1;
    block->addr = addr;
    block->length = length;
    block->offset = offset;
    block->transferredSize = 0;
    block->callback = callback;
    block->state = 1;

    while (!VirtualDI::Read(addr, length, offset)) {
        Clock::WaitMilliseconds(1000);
        while (!VirtualDI::Mount()) {
            Clock::WaitMilliseconds(1000);
        }
    }

    block->transferredSize = length;
    callback(length, block);
    block->state = 0;
    return true;
}

s32 DVDGetDriveStatus() {
    if (!VirtualDI::IsInit()) {
        return REPLACED(DVDGetDriveStatus)();
    }

    return DVD_STATE_END;
}

BOOL DVDCheckDisk() {
    if (!VirtualDI::IsInit()) {
        return REPLACED(DVDCheckDisk)();
    }

    return true;
}
