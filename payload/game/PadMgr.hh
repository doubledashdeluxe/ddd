#pragma once

#include <payload/Replace.hh>
#include <portable/Types.hh>

class PadRecorder;

class PadMgr {
public:
    struct KartPadData {
        s8 stickX : 5;
        s8 stickY : 3;
        u8 buttons;
    };
    size_assert(KartPadData, 0x2);

    static void SetRecorder(PadRecorder *recorder);
    static void Framework();
    static void REPLACED(GetPadData)(u8 port, bool remote, KartPadData *data);
    REPLACE static void GetPadData(u8 port, bool remote, KartPadData *data);

private:
    PadMgr();

    REPLACE static void ProcessKartPad();

    static BOOL s_kartPadInput;
    static PadRecorder *s_recorder;
};
size_assert(PadMgr, 0x1);
