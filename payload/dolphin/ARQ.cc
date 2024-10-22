extern "C" {
#include "ARQ.h"
}

#include <common/DCache.hh>
#include <common/Memory.hh>
#include <common/Platform.hh>

extern "C" {
#include <string.h>
}

extern "C" void ARQInit() {
    if (Platform::IsGameCube()) {
        REPLACED(ARQInit)();
    }
}

extern "C" void ARQPostRequest(ARQRequest *task, u32 owner, u32 type, u32 priority, u32 source,
        u32 dest, u32 length, ARQCallback callback) {
    if (Platform::IsGameCube()) {
        REPLACED(ARQPostRequest)(task, owner, type, priority, source, dest, length, callback);
        return;
    }

    task->next = nullptr;
    task->owner = owner;
    task->type = type;
    task->priority = priority;
    task->source = source + length;
    task->dest = dest + length;
    task->length = 0;
    task->callback = callback;

    DCache::Invalidate(Memory::PhysicalToVirtual<void>(source), length);
    memcpy(Memory::PhysicalToVirtual<void>(dest), Memory::PhysicalToVirtual<void>(source), length);
    DCache::Flush(Memory::PhysicalToVirtual<void>(dest), length);
    if (callback) {
        callback(task);
    }
}
