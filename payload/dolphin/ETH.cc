extern "C" {
#include "ETH.h"
}

#include <payload/VirtualETH.hh>

static VirtualETH *s_virtualETH = nullptr;

s32 ETHInit(s32 mode) {
    s_virtualETH = VirtualETH::Instance();
    s32 result = s_virtualETH->init(mode);
    if (result >= 0) {
        return result;
    }

    s_virtualETH = nullptr;
    return REPLACED(ETHInit)(mode);
}

void ETHGetMACAddr(u8 *macaddr) {
    if (s_virtualETH) {
        s_virtualETH->getMACAddr(macaddr);
        return;
    }

    REPLACED(ETHGetMACAddr)(macaddr);
}

void ETHSetRecvCallback(ETHCallback0 callback0, ETHCallback1 callback1) {
    if (s_virtualETH) {
        s_virtualETH->setRecvCallback(callback0, callback1);
        return;
    }

    REPLACED(ETHSetRecvCallback)(callback0, callback1);
}

BOOL ETHGetLinkStateAsync(BOOL *status) {
    if (s_virtualETH) {
        return s_virtualETH->getLinkStateAsync(status);
    }

    return REPLACED(ETHGetLinkStateAsync)(status);
}

void ETHSetProtoType(u16 *array, s32 num) {
    if (s_virtualETH) {
        s_virtualETH->setProtoType(array, num);
        return;
    }

    REPLACED(ETHSetProtoType)(array, num);
}

void ETHSendAsync(void *addr, s32 length, ETHCallback2 callback2) {
    if (s_virtualETH) {
        s_virtualETH->sendAsync(addr, length, callback2);
        return;
    }

    REPLACED(ETHSendAsync)(addr, length, callback2);
}

void ETHAddMulticastAddress(const u8 macaddr[6]) {
    if (s_virtualETH) {
        s_virtualETH->addMulticastAddress(macaddr);
        return;
    }

    REPLACED(ETHAddMulticastAddress)(macaddr);
}

void ETHClearMulticastAddresses() {
    if (s_virtualETH) {
        s_virtualETH->clearMulticastAddresses();
        return;
    }

    REPLACED(ETHClearMulticastAddresses)();
}
