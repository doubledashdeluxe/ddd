#include "VI.hh"

#include <common/DCache.hh>

extern "C" volatile u16 vtr;
extern "C" volatile u16 dcr;
extern "C" volatile u32 vto;
extern "C" volatile u32 vte;
extern "C" volatile u32 tfbl;
extern "C" volatile u32 bfbl;
extern "C" volatile u16 hsw;
extern "C" volatile u16 hsr;
extern "C" volatile u16 visel;

void VI::Init() {
    bool isProgressive = IsProgressive();
    bool isNtsc = (dcr >> 8 & 3) == 0;
    s_xfbWidth = 640;
    s_xfbHeight = isProgressive || isNtsc ? 480 : 574;
    s_xfb = reinterpret_cast<u32 *>(0x91000000);
    for (u16 y = 0; y < s_xfbHeight; y++) {
        for (u16 x = 0; x < s_xfbWidth; x++) {
            WriteGrayscaleToXFB(x, y, 16);
        }
    }
    FlushXFB();
    vtr = s_xfbHeight << (3 + isProgressive) | (vtr & 0xf);
    if (isProgressive) {
        vto = 0x6 << 16 | 0x30;
        vte = 0x6 << 16 | 0x30;
    } else if (isNtsc) {
        vto = 0x3 << 16 | 0x18;
        vte = 0x2 << 16 | 0x19;
    } else {
        vto = 0x1 << 16 | 0x23;
        vte = 0x0 << 16 | 0x24;
    }
    hsw = 0x2828;
    hsr = 0x10f5;
    tfbl = 1 << 28 | reinterpret_cast<u32>(s_xfb) >> 5;
    bfbl = 1 << 28 | reinterpret_cast<u32>(s_xfb) >> 5;
}

bool VI::IsProgressive() {
    return visel & 1 || dcr & 4;
}

u16 VI::GetXFBWidth() {
    return s_xfbWidth;
}

u16 VI::GetXFBHeight() {
    return s_xfbHeight;
}

u8 VI::ReadGrayscaleFromXFB(u16 x, u16 y) {
    if (x > s_xfbWidth || y > s_xfbHeight) {
        return 16;
    }

    u32 val = s_xfb[y * (s_xfbWidth / 2) + x / 2];
    if (x & 1) {
        return val >> 8;
    } else {
        return val >> 24;
    }
}

void VI::WriteGrayscaleToXFB(u16 x, u16 y, u8 intensity) {
    if (x > s_xfbWidth || y > s_xfbHeight) {
        return;
    }

    u32 *val = &s_xfb[y * (s_xfbWidth / 2) + x / 2];
    u8 y0 = *val >> 24;
    u8 y1 = *val >> 8;
    if (x & 1) {
        y1 = intensity;
    } else {
        y0 = intensity;
    }
    *val = y0 << 24 | 127 << 16 | y1 << 8 | 127;
}

void VI::FlushXFB() {
    DCache::Store(s_xfb, 320 * 574 * sizeof(u32));
}

u16 VI::s_xfbWidth;
u16 VI::s_xfbHeight;
u32 *VI::s_xfb;
