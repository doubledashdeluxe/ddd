#pragma once

#include "dolphin/IPEther.h"

#include <payload/Replace.h>

void REPLACED(IPGetMacAddr)(const IPInterface *interface, u8 macAddr[6]);
REPLACE void IPGetMacAddr(const IPInterface *interface, u8 macAddr[6]);
void REPLACED(IPGetNetmask)(const IPInterface *interface, u8 netmask[4]);
REPLACE void IPGetNetmask(const IPInterface *interface, u8 netmask[4]);
void REPLACED(IPGetAddr)(const IPInterface *interface, u8 addr[4]);
REPLACE void IPGetAddr(const IPInterface *interface, u8 addr[4]);
void REPLACED(IPGetAlias)(const IPInterface *interface, u8 alias[4]);
REPLACE void IPGetAlias(const IPInterface *interface, u8 alias[4]);
void REPLACED(IPGetLinkState)(const IPInterface *interface, s32 *state);
REPLACE void IPGetLinkState(const IPInterface *interface, s32 *state);
s32 REPLACED(IPGetConfigError)(const IPInterface *interface);
REPLACE s32 IPGetConfigError(const IPInterface *interface);
s32 REPLACED(IPClearConfigError)(IPInterface *interface);
REPLACE s32 IPClearConfigError(IPInterface *interface);
