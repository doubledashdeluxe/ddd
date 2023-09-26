#include "Arena.hh"

#include "common/Align.hh"

MEM1Arena &MEM1Arena::Instance() {
    return s_instance;
}

MEM1Arena::MEM1Arena() {}

MEM1Arena::~MEM1Arena() {}

MEM1Arena MEM1Arena::s_instance;

void *MEM2Arena::alloc(size_t size, s32 align) {
    if (align >= 0) {
        m_lo = AlignUp<u32>(m_lo, align);
        void *ptr = reinterpret_cast<void *>(m_lo);
        m_lo += size;
        return ptr;
    } else {
        m_hi = AlignDown<u32>(m_hi, -align);
        void *ptr = reinterpret_cast<void *>(m_hi);
        m_hi -= size;
        return ptr;
    }
}

MEM2Arena &MEM2Arena::Instance() {
    return s_instance;
}

MEM2Arena::MEM2Arena() : m_lo(0x91000000), m_hi(0x93400000) {}

MEM2Arena::~MEM2Arena() {}

MEM2Arena MEM2Arena::s_instance;

void *operator new(size_t size, Arena &arena, s32 align) {
    return arena.alloc(size, align);
}

void operator delete(void * /* ptr */, Arena & /* arena */) {}
