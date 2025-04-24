#include "VI.hh"

#include "cube/Arena.hh"
#include "cube/DCache.hh"
#include "cube/Memory.hh"

#include <portable/Align.hh>

extern "C" u32 videoMode;

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

bool VI::isProgressive() const {
    return m_isProgressive;
}

u16 VI::getXFBWidth() const {
    return m_xfbWidth;
}

u16 VI::getXFBHeight() const {
    return m_xfbHeight;
}

u32 VI::getXFBSize() const {
    return m_xfbSize;
}

u32 *VI::getXFB() const {
    return m_xfb;
}

VI::Color VI::readFromXFB(u16 x, u16 y) {
    if (x >= m_xfbWidth || y >= m_xfbHeight) {
        return Color::Black;
    }

    u32 val = m_xfb[y * (m_xfbWidth / 2) + x / 2];
    Color color;
    color.y0 = val >> 24;
    color.u = val >> 16;
    color.y1 = val >> 8;
    color.v = val >> 0;
    return color;
}

void VI::writeToXFB(u16 x, u16 y, Color color) {
    if (x >= m_xfbWidth || y >= m_xfbHeight) {
        return;
    }

    u32 val = color.y0 << 24 | color.u << 16 | color.y1 << 8 | color.v;
    m_xfb[y * (m_xfbWidth / 2) + x / 2] = val;
}

void VI::Init() {
    s_instance = new (MEM1Arena::Instance(), -0x4) VI;
}

void VI::Init(VI *instance) {
    s_instance = instance;
}

VI *VI::Instance() {
    return s_instance;
}

VI::VI() {
    bool isNtsc = (dcr >> 8 & 3) == 0;
    m_isProgressive = isNtsc && (visel & 1 || dcr & 4);
    m_xfbWidth = 608;
    m_xfbHeight = isNtsc ? 448 : 538;
    m_xfbSize = m_xfbHeight * AlignUp(m_xfbWidth, 16) * sizeof(u16);
    m_xfb = reinterpret_cast<u32 *>(0xc0360000);
    for (u16 y = 0; y < m_xfbHeight; y++) {
        for (u16 x = 0; x < m_xfbWidth; x++) {
            writeToXFB(x, y, Color::Black);
        }
    }
    vtr = (m_xfbHeight << 3 | (5 + isNtsc) << 0) << m_isProgressive;
    if (m_isProgressive) {
        vto = 0x26 << 16 | 0x50;
        vte = 0x26 << 16 | 0x50;
    } else if (isNtsc) {
        vto = 0x13 << 16 | 0x28;
        vte = 0x12 << 16 | 0x29;
    } else {
        vto = 0x13 << 16 | 0x35;
        vte = 0x12 << 16 | 0x36;
    }
    hsw = m_xfbWidth << 4 | m_xfbWidth >> (3 + m_isProgressive);
    hsr = isNtsc ? 0x10ea : 0x10e9;
    tfbl = 1 << 28 | Memory::UncachedToPhysical(m_xfb) >> 5;
    bfbl = 1 << 28 | Memory::UncachedToPhysical(m_xfb + (m_xfbWidth / 2) * !m_isProgressive) >> 5;
    videoMode = !isNtsc;
}

VI *VI::s_instance = nullptr;
