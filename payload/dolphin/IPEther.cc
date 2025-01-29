extern "C" {
#include "IPEther.h"

#include "dolphin/ETH.h"
#include "dolphin/IPIgmp.h"
#include "dolphin/IPRoute.h"
#include "dolphin/OSAlarm.h"
#include "dolphin/OSTime.h"
}

#include <common/Array.hh>
#include <common/Clock.hh>

extern "C" IPInterface IFDefault;
extern "C" OSAlarm LinkAlarm;

extern "C" void LinkCheckHandler(OSAlarm *alarm, OSContext *context);

BOOL IFInit(s32 mode) {
    static bool isInit = false;
    if (!isInit) {
        isInit = true;
        bool result = REPLACED(IFInit)(mode);
        if (IFDefault.mode == ETH_MODE_UNKNOWN) {
            OSCreateAlarm(&LinkAlarm);
        }
        return result;
    }

    OSCancelAlarm(&LinkAlarm);
    if (ETHInit(mode) >= 0) {
        IFDefault.mode = mode;
        ETHGetMACAddr(IFDefault.macaddr);
        static Array<u16, 4> protocols;
        protocols[0] = 0x0800; // IPv4
        protocols[1] = 0x0806; // ARP
        protocols[2] = 0x8863; // PPPoE Discovery Stage
        protocols[3] = 0x8864; // PPPoE Session Stage
        ETHSetProtoType(protocols.values(), protocols.count());
        IFMute(false);
        s64 start = OSGetTime();
        s64 period = Clock::MillisecondsToTicks(250);
        OSSetPeriodicAlarm(&LinkAlarm, start, period, LinkCheckHandler);
        IGMPInit(&IFDefault);
    } else {
        IFMute(true);
        IFDefault.mode = ETH_MODE_UNKNOWN;
    }
    return true;
}
