#pragma once

#include <jsystem/JKRHeap.hh>

class GameApp {
public:
    GameApp(u32 size, const char *name, void *ptr);
    virtual ~GameApp();
    virtual void draw();
    virtual void calc();
    virtual void reset();

protected:
    JKRHeap *m_heap;

private:
    u8 _8[0xc - 0x8];
};
size_assert(GameApp, 0xc);
