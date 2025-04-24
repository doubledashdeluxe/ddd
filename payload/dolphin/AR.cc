extern "C" {
#include "AR.h"
}

#include <cube/Platform.hh>
#include <payload/Lock.hh>

static u32 s_stackPointer;
static u32 s_freeBlocks;
static u32 *s_blockLength;

extern "C" u32 ARInit(u32 *stack_index_addr, u32 num_entries) {
    if (Platform::IsGameCube()) {
        return REPLACED(ARInit)(stack_index_addr, num_entries);
    }

    s_stackPointer = 0x4000;
    s_freeBlocks = num_entries;
    s_blockLength = stack_index_addr;
    return s_stackPointer;
}

extern "C" u32 ARAlloc(u32 length) {
    if (Platform::IsGameCube()) {
        return REPLACED(ARAlloc)(length);
    }

    Lock<NoInterrupts> lock;
    u32 block = 0x10000000 + s_stackPointer;
    s_stackPointer += length;
    *s_blockLength = length;
    s_freeBlocks--;
    return block;
}

extern "C" u32 ARGetSize() {
    if (Platform::IsGameCube()) {
        return REPLACED(ARGetSize)();
    }

    return 0x1000000;
}
