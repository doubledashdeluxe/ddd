#pragma once

#include <portable/Types.h>

void StackCanary_Init(void);
u32 StackCanary_Canary(void);
