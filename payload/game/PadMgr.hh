#pragma once

#include <common/Types.hh>
#include <payload/Replace.hh>

class PadMgr {
public:
    struct KartPadData {
        s8 stickX : 5;
        s8 stickY : 3;
        u8 buttons;
    };
    size_assert(KartPadData, 0x2);

    static void Framework();
    static void REPLACED(GetPadData)(u8 port, bool remote, KartPadData *data);
    REPLACE static void GetPadData(u8 port, bool remote, KartPadData *data);

private:
    PadMgr();
};
size_assert(PadMgr, 0x1);
