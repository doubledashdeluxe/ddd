#include "Patcher.hh"

#include <cube/DCache.hh>
#include <cube/ICache.hh>

extern "C" u32 Config24MB[];
extern "C" u32 Config48MB[];
extern "C" u32 RealMode[];
extern "C" u32 __LCEnable[];

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

    Wipe(Config24MB, 32);
    Wipe(Config48MB, 32);
    Wipe(RealMode, 6);
    Wipe(__LCEnable, 50);
}

void Patcher::Wipe(u32 *insts, u32 instCount) {
    for (u32 i = 0; i < instCount; i++) {
        uintptr_t address = reinterpret_cast<uintptr_t>(&insts[i]);
        u32 *inst = reinterpret_cast<u32 *>(address | 0x40000000);
        *inst = 0x4e800020;
        DCache::Flush(inst, sizeof(*inst));
        ICache::Invalidate(inst, sizeof(*inst));
    }
}
