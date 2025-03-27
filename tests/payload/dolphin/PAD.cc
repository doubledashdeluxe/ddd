extern "C" {
#include <dolphin/PAD.h>
}
#include <payload/WUP028.hh>
#define SNITCH_IMPLEMENTATION
#include <snitch/snitch_all.hpp>

#include <cstring>

static PADStatus siStatus[4];
static u32 siRumbleSupported;
static PADStatus wup028Status[4];
static u32 wup028RumbleSupported;

u32 REPLACED(PADRead)(PADStatus *status) {
    for (u32 i = 0; i < 4; i++) {
        status[i] = siStatus[i];
    }
    return siRumbleSupported;
}

u32 WUP028::Read(PADStatus *status) {
    for (u32 i = 0; i < 4; i++) {
        status[i] = wup028Status[i];
    }
    return wup028RumbleSupported;
}

static void Setup(PADStatus (&status)[4], size_t offset) {
    for (size_t i = 0; i < 4; i++) {
        status[i].button = i + offset;
        status[i].stickX = i + offset;
        status[i].stickY = i + offset;
        status[i].substickX = i + offset;
        status[i].substickY = i + offset;
        status[i].triggerLeft = i + offset;
        status[i].triggerRight = i + offset;
        status[i].analogA = i + offset;
        status[i].analogB = i + offset;
    }
}

TEST_CASE("PADRead") {
    Setup(siStatus, 0);
    Setup(wup028Status, 4);
    siStatus[0].err = PAD_ERR_NONE;
    siStatus[1].err = PAD_ERR_NONE;
    siStatus[2].err = PAD_ERR_NO_CONTROLLER;
    siStatus[3].err = PAD_ERR_NO_CONTROLLER;
    siRumbleSupported = 1 << (31 - 1);
    wup028Status[0].err = PAD_ERR_NONE;
    wup028Status[1].err = PAD_ERR_NO_CONTROLLER;
    wup028Status[2].err = PAD_ERR_NONE;
    wup028Status[3].err = PAD_ERR_NO_CONTROLLER;
    wup028RumbleSupported = 1 << (31 - 0);

    PADStatus status[4];
    u32 rumbleSupported = PADRead(status);

    CHECK(status[0].err == PAD_ERR_NONE);
    CHECK(status[1].err == PAD_ERR_NONE);
    CHECK(status[2].err == PAD_ERR_NONE);
    CHECK(status[3].err == PAD_ERR_NO_CONTROLLER);
    CHECK_FALSE(memcmp(&status[0], &wup028Status[0], sizeof(PADStatus)));
    CHECK_FALSE(memcmp(&status[1], &siStatus[1], sizeof(PADStatus)));
    CHECK_FALSE(memcmp(&status[2], &wup028Status[2], sizeof(PADStatus)));
    CHECK(rumbleSupported == (1 << (31 - 0) | 1 << (31 - 1)));
}
