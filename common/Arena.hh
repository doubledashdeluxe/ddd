#pragma once

#include <common/Types.hh>

class Arena {
public:
    virtual void *alloc(size_t size, s32 align) = 0;
};

class MEM1Arena : public Arena {
public:
    void *alloc(size_t size, s32 align);

    static MEM1Arena &Instance();

private:
    MEM1Arena();
    ~MEM1Arena();

    static MEM1Arena s_instance;
};

class MEM2Arena : public Arena {
public:
    void *alloc(size_t size, s32 align);

    static MEM2Arena &Instance();

private:
    MEM2Arena();
    ~MEM2Arena();

    u32 m_lo;
    u32 m_hi;

    static MEM2Arena s_instance;
};

void *operator new(size_t size, Arena &arena, s32 align);
void operator delete(void *ptr, Arena &arena);
