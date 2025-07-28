#pragma once

#include <jsystem/JKRHeap.hh>
#include <payload/Replace.hh>

class GameApp {
public:
    REPLACE GameApp(u32 size, const char *name, void *ptr);
    virtual ~GameApp();
    virtual void draw();
    virtual void calc();
    virtual void reset();

protected:
    JKRHeap *m_heap;

private:
    void *m_ptr;
};
size_assert(GameApp, 0xc);
