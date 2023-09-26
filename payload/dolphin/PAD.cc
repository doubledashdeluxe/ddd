extern "C" {
#include "PAD.h"
}

#include <payload/WUP028.hh>

extern "C" u32 PADRead(PADStatus *status) {
    PADStatus siStatus[4];
    u32 siRumbleSupported = REPLACED(PADRead)(siStatus);

    PADStatus wup028Status[4];
    u32 wup028RumbleSupported = WUP028::Read(wup028Status);

    u32 rumbleSupported = 0;
    for (u32 i = 0; i < 4; i++) {
        if (wup028Status[i].err != PAD_ERR_NONE) {
            status[i] = siStatus[i];
            rumbleSupported |= siRumbleSupported & (1 << (31 - i));
        } else {
            status[i] = wup028Status[i];
            rumbleSupported |= wup028RumbleSupported & (1 << (31 - i));
        }
    }
    return rumbleSupported;
}

extern "C" void PADControlMotor(int chan, u32 command) {
    REPLACED(PADControlMotor)(chan, command);
    WUP028::ControlMotor(chan, command);
}
