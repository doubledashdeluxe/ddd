#include "OS.h"

#include <payload/Replace.h>
#include <portable/Types.h>

extern OSExceptionHandler *OSExceptionTable;
extern u32 OSExceptionLocations[];

void OSExceptionVector();
void OSDefaultExceptionHandler(u8 exception, OSContext *context);

REPLACE void OSExceptionInit(void) {
    uintptr_t vector = (uintptr_t)OSExceptionVector | 0xc0000000;
    for (u32 i = 0; i < 15; i++) {
        uintptr_t location = OSExceptionLocations[i] | 0xc0000000;
        u32 *insts = (u32 *)location;
        insts[0] = 0x7c9043a6;  // mtsprg0 r4
        insts[1] = 0x3c800040;  // lis r4, 0x40
        insts[2] = 0xa08400c0;  // lhz r4, 0xc0 (r4)
        insts[3] = 0x7c7143a6;  // mtsprg1 r3
        insts[4] = 0x7c600026;  // mfcr r3
        insts[5] = 0x2c040040;  // cmpwi r4, 0x40
        insts[6] = 0x41800000;  // blt 0
        insts[7] = 0x3c800040;  // lis r4, 0x40
        insts[8] = 0x808400c0;  // lwz r4, 0xc0 (r4)
        insts[9] = 0x90640080;  // stw r3, 0x80 (r4)
        insts[10] = 0x7c7142a6; // mfsprg1 r3
        for (u32 i = 11; i < 18; i++) {
            insts[i] = ((u32 *)vector)[i - 11 + 2];
        }
        for (u32 i = 18; i < 33; i++) {
            insts[i] = ((u32 *)vector)[i - 18 + 11];
        }
        insts[33] = 0x38600000 | i; // li r3, i
        insts[34] = 0x3c800040;     // lis r4, 0x40
        insts[35] = 0x808400d4;     // lwz r4, 0xd4 (r4)
        for (u32 i = 36; i < 46; i++) {
            insts[i] = ((u32 *)vector)[i - 36 + 28];
        }
    }

    OSExceptionTable = (OSExceptionHandler *)0xc0003000;
    for (u32 i = 0; i < 15; i++) {
        OSSetExceptionHandler(i, OSDefaultExceptionHandler);
    }
}
