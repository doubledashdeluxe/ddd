extern "C" {
#include "DVD.h"
}

#include <cube/Clock.hh>
#include <cube/Memory.hh>
#include <cube/VirtualDI.hh>
#include <payload/Lock.hh>
#include <payload/Mutex.hh>
#include <payload/Replace.hh>

#ifdef __CWCC__
static Mutex s_mutex;
#endif

extern "C" void REPLACED(Read)(void *addr, s32 length, s32 offset, DVDCBCallback callback);
extern "C" REPLACE void Read(void *addr, s32 length, s32 offset, DVDCBCallback callback) {
    addr = reinterpret_cast<void *>(Memory::CachedToPhysical(addr));
    REPLACED(Read)(addr, length, offset, callback);
}

extern "C" BOOL REPLACED(DVDLowReadDiskID)(void *addr, DVDCBCallback callback);
extern "C" REPLACE BOOL DVDLowReadDiskID(void *addr, DVDCBCallback callback) {
    addr = reinterpret_cast<void *>(Memory::CachedToPhysical(addr));
    return REPLACED(DVDLowReadDiskID)(addr, callback);
}

extern "C" BOOL REPLACED(DVDLowInquiry)(void *addr, DVDCBCallback callback);
extern "C" REPLACE BOOL DVDLowInquiry(void *addr, DVDCBCallback callback) {
    addr = reinterpret_cast<void *>(Memory::CachedToPhysical(addr));
    return REPLACED(DVDLowInquiry)(addr, callback);
}

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
