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

const VI::Color VI::Color::Black = {16, 128, 16, 128};
const VI::Color VI::Color::White = {255, 128, 255, 128};

void VI::Init() {
    bool isProgressive = IsProgressive();
    bool isNtsc = (dcr >> 8 & 3) == 0;
    s_xfbWidth = 640;
    s_xfbHeight = isProgressive || isNtsc ? 480 : 574;
    s_xfb = reinterpret_cast<u32 *>(0x91000000);
    for (u16 y = 0; y < s_xfbHeight; y++) {
        for (u16 x = 0; x < s_xfbWidth; x++) {
            WriteToXFB(x, y, Color::Black);
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

VI::Color VI::ReadFromXFB(u16 x, u16 y) {
    if (x > s_xfbWidth || y > s_xfbHeight) {
        return Color::Black;
    }

    u32 val = s_xfb[y * (s_xfbWidth / 2) + x / 2];
    Color color = {val >> 24, val >> 16, val >> 8, val >> 0};
    return color;
}

void VI::WriteToXFB(u16 x, u16 y, Color color) {
    if (x > s_xfbWidth || y > s_xfbHeight) {
        return;
    }

    u32 val = color.y0 << 24 | color.u << 16 | color.y1 << 8 | color.v;
    s_xfb[y * (s_xfbWidth / 2) + x / 2] = val;
}

void VI::FlushXFB() {
    DCache::Store(s_xfb, 320 * 574 * sizeof(u32));
}

u16 VI::s_xfbWidth;
u16 VI::s_xfbHeight;
u32 *VI::s_xfb;
