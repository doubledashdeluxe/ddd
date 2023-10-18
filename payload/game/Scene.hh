#pragma once

#include <common/Types.hh>
#include <jsystem/JKRArchive.hh>
#include <jsystem/JKRHeap.hh>

class Scene {
private:
    u8 _0[0x8 - 0x0];

public:
    Scene(JKRArchive *archive, JKRHeap *heap);
    virtual ~Scene();
    virtual void init() = 0;
    virtual void draw() = 0;
    virtual void calc() = 0;
};
size_assert(Scene, 0xc);
