extern "C" {
#include "AR.h"
}

#include <payload/Lock.hh>

static u32 s_stackPointer;
static u32 s_freeBlocks;
static u32 *s_blockLength;

extern "C" u32 ARInit(u32 *stack_index_addr, u32 num_entries) {
    s_stackPointer = 0x10004000;
    s_freeBlocks = num_entries;
    s_blockLength = stack_index_addr;
    return s_stackPointer;
}

extern "C" u32 ARAlloc(u32 length) {
    Lock<NoInterrupts> lock;
    u32 block = s_stackPointer;
    s_stackPointer += length;
    *s_blockLength = length;
    s_freeBlocks--;
    return block;
}

extern "C" u32 ARGetSize() {
    return 0x1000000;
}
