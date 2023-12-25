extern "C" {
#include <dolphin/PAD.h>
}
#include <lest.hpp>
#include <payload/WUP028.hh>

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

static lest::tests specification;

#define CASE(name) lest_CASE(specification, name)

CASE("PADRead") {
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

    EXPECT(status[0].err == PAD_ERR_NONE);
    EXPECT(status[1].err == PAD_ERR_NONE);
    EXPECT(status[2].err == PAD_ERR_NONE);
    EXPECT(status[3].err == PAD_ERR_NO_CONTROLLER);
    EXPECT(!memcmp(&status[0], &wup028Status[0], sizeof(PADStatus)));
    EXPECT(!memcmp(&status[1], &siStatus[1], sizeof(PADStatus)));
    EXPECT(!memcmp(&status[2], &wup028Status[2], sizeof(PADStatus)));
    EXPECT(rumbleSupported == (1 << (31 - 0) | 1 << (31 - 1)));
}

int main(int argc, char *argv[]) {
    return lest::run(specification, argc, argv, std::cerr);
}
