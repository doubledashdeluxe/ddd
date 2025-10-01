#pragma once

#include <payload/Replace.h>
#include <portable/Types.h>

enum {
    ETH_MODE_AUTO = 0,
    ETH_MODE_100FULL = 1,
    ETH_MODE_100HALF = 2,
    ETH_MODE_10FULL = 3,
    ETH_MODE_10HALF = 4,
    ETH_MODE_UNKNOWN = 5,
};

typedef void *(*ETHCallback0)(u16 type, s32 len, u8 lrps);
typedef void (*ETHCallback1)(u8 *addr, s32 len);
typedef void (*ETHCallback2)(u8 ltps);

s32 REPLACED(ETHInit)(s32 mode);
REPLACE s32 ETHInit(s32 mode);
void REPLACED(ETHGetMACAddr)(u8 macaddr[6]);
REPLACE void ETHGetMACAddr(u8 macaddr[6]);
void REPLACED(ETHSetRecvCallback)(ETHCallback0 callback0, ETHCallback1 callback1);
REPLACE void ETHSetRecvCallback(ETHCallback0 callback0, ETHCallback1 callback1);
BOOL REPLACED(ETHGetLinkStateAsync)(BOOL *status);
REPLACE BOOL ETHGetLinkStateAsync(BOOL *status);
void REPLACED(ETHSetProtoType)(u16 *array, s32 num);
REPLACE void ETHSetProtoType(u16 *array, s32 num);
void REPLACED(ETHSendAsync)(void *addr, s32 length, ETHCallback2 callback2);
REPLACE void ETHSendAsync(void *addr, s32 length, ETHCallback2 callback2);
void REPLACED(ETHAddMulticastAddress)(const u8 macaddr[6]);
REPLACE void ETHAddMulticastAddress(const u8 macaddr[6]);
void REPLACED(ETHClearMulticastAddresses)(void);
REPLACE void ETHClearMulticastAddresses(void);

const char *ETHName(void);
