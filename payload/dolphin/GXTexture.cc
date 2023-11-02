extern "C" {
#include "GXTexture.h"
}

extern "C" void GXInitTlutObj(GXTlutObj *tlut_obj, void *lut, s32 fmt, u16 n_entries) {
    tlut_obj->fmt = fmt << 10;
    // The Dolphin SDK doesn't reset the middle bits, which is an issue in Wii mode as they are not
    // masked. Reference (Dolphin Emulator): https://is.gd/ghHrLe
    tlut_obj->lut = 0x64 << 24 | (reinterpret_cast<uintptr_t>(lut) & 0x1ffffff) >> 5;
    tlut_obj->n_entries = n_entries;
}
