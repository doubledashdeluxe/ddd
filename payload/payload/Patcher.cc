#include "Patcher.hh"

#include <common/DCache.hh>
#include <common/ICache.hh>

extern "C" u32 Config24MB[];
extern "C" u32 Config48MB[];
extern "C" u32 RealMode[];

void Patcher::Run() {
    for (size_t i = 0; i < PatchCount; i++) {
        const Patch &patch = Patches[i];
        uintptr_t fromAddress = reinterpret_cast<uintptr_t>(patch.from);
        uintptr_t toAddress = reinterpret_cast<uintptr_t>(patch.to);
        uintptr_t thunkAddress = reinterpret_cast<uintptr_t>(patch.thunk);
        u32 *from = reinterpret_cast<u32 *>(fromAddress | 0x40000000);
        u32 *thunk = reinterpret_cast<u32 *>(thunkAddress | 0x40000000);

        if (patch.thunk) {
            thunk[0] = *from;
            thunk[1] = 0x12 << 26 | ((fromAddress - thunkAddress) & 0x3fffffc);
            DCache::Flush(patch.thunk, sizeof(u32) * 2);
            ICache::Invalidate(patch.thunk, sizeof(u32) * 2);
        }

        u32 branchInst = 0x12 << 26 | ((toAddress - fromAddress) & 0x3fffffc);
        *from = branchInst;
        DCache::Flush(patch.from, sizeof(branchInst));
        ICache::Invalidate(patch.from, sizeof(branchInst));
    }

    for (u32 i = 0; i < 32; i++) {
        uintptr_t address = reinterpret_cast<uintptr_t>(&Config24MB[i]);
        u32 *inst = reinterpret_cast<u32 *>(address | 0x40000000);
        *inst = 0x4e800020;
        DCache::Flush(inst, sizeof(*inst));
        ICache::Invalidate(inst, sizeof(*inst));
    }
    for (u32 i = 0; i < 32; i++) {
        uintptr_t address = reinterpret_cast<uintptr_t>(&Config48MB[i]);
        u32 *inst = reinterpret_cast<u32 *>(address | 0x40000000);
        *inst = 0x4e800020;
        DCache::Flush(inst, sizeof(*inst));
        ICache::Invalidate(inst, sizeof(*inst));
    }
    for (u32 i = 0; i < 6; i++) {
        uintptr_t address = reinterpret_cast<uintptr_t>(&RealMode[i]);
        u32 *inst = reinterpret_cast<u32 *>(address | 0x40000000);
        *inst = 0x4e800020;
        DCache::Flush(inst, sizeof(*inst));
        ICache::Invalidate(inst, sizeof(*inst));
    }
}
