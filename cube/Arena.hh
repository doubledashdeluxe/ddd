#pragma once

#include <portable/Types.hh>

class Arena {
public:
    virtual void *alloc(size_t size, s32 align) = 0;
};

class MEM1Arena : public Arena {
public:
    void *alloc(size_t size, s32 align) override;

    static void Init();
    static MEM1Arena *Instance();

private:
    MEM1Arena();
    ~MEM1Arena();

    static MEM1Arena *s_instance;
    static u32 s_hi;
};

class MEM2Arena : public Arena {
public:
    void *alloc(size_t size, s32 align) override;

    static void Init(u32 lo, u32 hi);
    static MEM2Arena *Instance();

private:
    MEM2Arena(u32 lo, u32 hi);
    ~MEM2Arena();

    u32 m_lo;
    u32 m_hi;

    static MEM2Arena *s_instance;
};

void *operator new(size_t size, Arena *arena, s32 align);
