extern "C" {
#include "IPRoute.h"

#include "dolphin/IPSocket.h"
}

extern "C" {
#include <string.h>
}

void IPGetMacAddr(const IPInterface *interface, u8 macAddr[6]) {
    if (!SOIsVirtual()) {
        REPLACED(IPGetMacAddr)(interface, macAddr);
        return;
    }

    memset(macAddr, 0, 6);
    s32 optlen = 6;
    SOGetInterfaceOpt(SO_CONFIG_MAC_ADDRESS, macAddr, &optlen);
}

void IPGetNetmask(const IPInterface *interface, u8 netmask[4]) {
    if (!SOIsVirtual()) {
        REPLACED(IPGetNetmask)(interface, netmask);
        return;
    }

    u8 table[12];
    memset(table, 0, sizeof(table));
    s32 optlen = sizeof(table);
    SOGetInterfaceOpt(SO_CONFIG_IP_ADDR_TABLE, table, &optlen);
    memcpy(netmask, table + 4, 4);
}

void IPGetAddr(const IPInterface *interface, u8 addr[4]) {
    if (!SOIsVirtual()) {
        REPLACED(IPGetAddr)(interface, addr);
        return;
    }

    u8 table[12];
    memset(table, 0, sizeof(table));
    s32 optlen = sizeof(table);
    SOGetInterfaceOpt(SO_CONFIG_IP_ADDR_TABLE, table, &optlen);
    memcpy(addr, table + 0, 4);
}

void IPGetAlias(const IPInterface *interface, u8 alias[4]) {
    if (!SOIsVirtual()) {
        REPLACED(IPGetAlias)(interface, alias);
        return;
    }

    u8 table[24];
    memset(table, 0, sizeof(table));
    s32 optlen = sizeof(table);
    SOGetInterfaceOpt(SO_CONFIG_IP_ADDR_TABLE, table, &optlen);
    if (optlen == sizeof(table)) {
        memcpy(alias, table + 12, 4);
    } else {
        memcpy(alias, table + 0, 4);
    }
}

void IPGetLinkState(const IPInterface * /* interface */, s32 *state) {
    *state = 1;
}

s32 IPGetConfigError(const IPInterface *interface) {
    if (!SOIsRunning()) {
        return 0;
    }

    if (!SOIsVirtual()) {
        return REPLACED(IPGetConfigError)(interface);
    }

    s32 configError = 0;
    s32 optlen = sizeof(configError);
    SOGetInterfaceOpt(SO_CONFIG_ERROR, &configError, &optlen);
    return configError;
}

s32 IPClearConfigError(IPInterface *interface) {
    if (!SOIsVirtual()) {
        return REPLACED(IPClearConfigError)(interface);
    }

    s32 prevConfigError = IPGetConfigError(interface);
    s32 configError = 0;
    SOSetInterfaceOpt(SO_CONFIG_ERROR, &configError, sizeof(configError));
    return prevConfigError;
}
