#pragma once

#include <jsystem/J2DGraphContext.hh>
#include <jsystem/JKRArchive.hh>
#include <jsystem/JKRHeap.hh>
#include <portable/Types.hh>

class Scene {
protected:
    JKRArchive *m_archive;
    J2DGraphContext *m_graphContext;

public:
    Scene(JKRArchive *archive, JKRHeap *heap);
    virtual ~Scene();
    virtual void init() = 0;
    virtual void draw() = 0;
    virtual void calc() = 0;
};
size_assert(Scene, 0xc);
