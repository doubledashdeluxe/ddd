#include "J3DGD.hh"

#include <common/Memory.hh>

void J3DGDSetTexImgPtr(u32 texMapID, void *ptr) {
    ptr = reinterpret_cast<void *>(Memory::CachedToPhysical(ptr) | 0x80000000);
    REPLACED(J3DGDSetTexImgPtr)(texMapID, ptr);
}
