#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

u32 PPCMfmsr(void);
void PPCMtmsr(u32 msr);

u32 REPLACED(PPCMfhid2)(void);
REPLACE u32 PPCMfhid2(void);
