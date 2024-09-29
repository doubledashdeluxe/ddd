#include "Arena.hh"

#include "common/Align.hh"

void MEM1Arena::Init() {
    MEM1Arena arena;
    s_instance = new (&arena, -0x4) MEM1Arena(arena);
}

MEM1Arena *MEM1Arena::Instance() {
    return s_instance;
}

MEM1Arena::MEM1Arena() {}

MEM1Arena::~MEM1Arena() {}

MEM1Arena *MEM1Arena::s_instance = nullptr;

void *MEM2Arena::alloc(size_t size, s32 align) {
    if (align >= 0) {
        m_lo = AlignUp<u32>(m_lo, align);
        void *ptr = reinterpret_cast<void *>(m_lo);
        m_lo += size;
        return ptr;
    } else {
        m_hi -= size;
        m_hi = AlignDown<u32>(m_hi, -align);
        void *ptr = reinterpret_cast<void *>(m_hi);
        return ptr;
    }
}

void MEM2Arena::Init(u32 lo, u32 hi) {
    MEM2Arena arena(lo, hi);
    s_instance = new (&arena, 0x4) MEM2Arena(arena);
}

MEM2Arena *MEM2Arena::Instance() {
    return s_instance;
}

MEM2Arena::MEM2Arena(u32 lo, u32 hi) : m_lo(lo), m_hi(hi) {}

MEM2Arena::~MEM2Arena() {}

MEM2Arena *MEM2Arena::s_instance = nullptr;

void *operator new(size_t size, Arena *arena, s32 align) {
    return arena->alloc(size, align);
}
