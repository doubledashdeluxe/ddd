#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

void REPLACED(J3DGDSetTexImgPtr)(u32 texMapID, void *ptr);
REPLACE void J3DGDSetTexImgPtr(u32 texMapID, void *ptr);
void REPLACED(J3DGDSetTexImgPtrRaw)(u32 texMapID, u32 addr);
REPLACE void J3DGDSetTexImgPtrRaw(u32 texMapID, u32 addr);
