#pragma once

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

s32 ETHInit(s32 mode);
void ETHGetMACAddr(u8 macaddr[6]);
void ETHSetRecvCallback(ETHCallback0 callback0, ETHCallback1 callback1);
BOOL ETHGetLinkStateAsync(BOOL *status);
void ETHSetProtoType(u16 *array, s32 num);
void ETHSendAsync(void *addr, s32 length, ETHCallback2 callback2);
void ETHAddMulticastAddress(const u8 macaddr[6]);
void ETHClearMulticastAddresses(void);
