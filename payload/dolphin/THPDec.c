#include "THPDec.h"

extern u8 *__THPLCWork512[3];
extern u8 *__THPLCWork640[3];

static void THPSetFakeLCBuffer512(u8 fakeLCBuffer[THP_FAKE_LC_BUFFER_SIZE]) {
    __THPLCWork512[0] = fakeLCBuffer;
    fakeLCBuffer += 0x2000;
    __THPLCWork512[1] = fakeLCBuffer;
    fakeLCBuffer += 0x800;
    __THPLCWork512[2] = fakeLCBuffer;
    fakeLCBuffer += 0x200;
}

static void THPSetFakeLCBuffer640(u8 fakeLCBuffer[THP_FAKE_LC_BUFFER_SIZE]) {
    __THPLCWork640[0] = fakeLCBuffer;
    fakeLCBuffer += 0x2800;
    __THPLCWork640[1] = fakeLCBuffer;
    fakeLCBuffer += 0xa00;
    __THPLCWork640[2] = fakeLCBuffer;
    fakeLCBuffer += 0xa00;
}

void THPInit(void) {}

void THPSetFakeLCBuffer(u8 fakeLCBuffer[THP_FAKE_LC_BUFFER_SIZE]) {
    THPSetFakeLCBuffer512(fakeLCBuffer);
    THPSetFakeLCBuffer640(fakeLCBuffer);
}
