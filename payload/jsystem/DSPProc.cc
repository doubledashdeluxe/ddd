#include <common/Memory.hh>
#include <payload/Replace.hh>

void REPLACED(DsetupTable)(u32 r3, u32 r4, u32 r5, u32 r6, u32 r7);
REPLACE void DsetupTable(u32 r3, u32 r4, u32 r5, u32 r6, u32 r7) {
    r4 = Memory::CachedToPhysical(reinterpret_cast<void *>(r4)) | 0x80000000;
    r5 = Memory::CachedToPhysical(reinterpret_cast<void *>(r5)) | 0x80000000;
    r6 = Memory::CachedToPhysical(reinterpret_cast<void *>(r6)) | 0x80000000;
    r7 = Memory::CachedToPhysical(reinterpret_cast<void *>(r7)) | 0x80000000;
    REPLACED(DsetupTable)(r3, r4, r5, r6, r7);
}

void REPLACED(DsyncFrame)(u32 r3, u32 r4, u32 r5);
REPLACE void DsyncFrame(u32 r3, u32 r4, u32 r5) {
    r4 = Memory::CachedToPhysical(reinterpret_cast<void *>(r4)) | 0x80000000;
    r5 = Memory::CachedToPhysical(reinterpret_cast<void *>(r5)) | 0x80000000;
    REPLACED(DsyncFrame)(r3, r4, r5);
}
