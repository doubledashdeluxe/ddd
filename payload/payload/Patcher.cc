#include "Patcher.hh"

#include <common/DCache.hh>
#include <common/ICache.hh>

extern "C" {
#include <string.h>
}

void Patcher::Run() {
    for (size_t i = 0; i < PatchCount; i++) {
        const Patch &patch = Patches[i];
        u32 fromAddress = reinterpret_cast<u32>(patch.from);
        u32 toAddress = reinterpret_cast<u32>(patch.to);
        u32 thunkAddress = reinterpret_cast<u32>(patch.thunk);

        if (patch.thunk) {
            patch.thunk[0] = *reinterpret_cast<u32 *>(patch.from);
            patch.thunk[1] = 0x12 << 26 | ((fromAddress - thunkAddress) & 0x3fffffc);
            DCache::Flush(patch.thunk, sizeof(u32) * 2);
            ICache::Invalidate(patch.thunk, sizeof(u32) * 2);
        }

        u32 branchInst = 0x12 << 26 | ((toAddress - fromAddress) & 0x3fffffc);
        memcpy(patch.from, &branchInst, sizeof(branchInst));
        DCache::Flush(patch.from, sizeof(branchInst));
        ICache::Invalidate(patch.from, sizeof(branchInst));
    }
}
