extern "C" {
#include "GXTexture.h"
}

#include <common/Memory.hh>

extern "C" void GXInitTexObj(GXTexObj *obj, void *image_ptr, u16 width, u16 height, s32 format,
        s32 wrap_s, s32 wrap_t, BOOL mipmap) {
    REPLACED(GXInitTexObj)(obj, image_ptr, width, height, format, wrap_s, wrap_t, mipmap);
    obj->image_ptr = (Memory::CachedToPhysical(image_ptr) & 0x13ffffff) >> 5;
}

extern "C" void GXInitTlutObj(GXTlutObj *tlut_obj, void *lut, s32 fmt, u16 n_entries) {
    tlut_obj->fmt = fmt << 10;
    // The Dolphin SDK doesn't reset the middle bits, which is an issue in Wii mode as they are not
    // masked. Reference (Dolphin Emulator): https://is.gd/ghHrLe
    tlut_obj->lut = 0x64 << 24 | (Memory::CachedToPhysical(lut) & 0x13ffffff) >> 5;
    tlut_obj->n_entries = n_entries;
}
