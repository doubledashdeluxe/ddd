#pragma once

#include "game/Scene.hh"
#include "game/SceneType.hh"

#include <payload/Replace.hh>

class SceneFactory {
public:
    SceneFactory();
    ~SceneFactory();

    Scene *REPLACED(createScene)(s32 sceneType, JKRHeap *heap);
    REPLACE Scene *createScene(s32 sceneType, JKRHeap *heap);

private:
    JKRArchive *m_archives[0xf];
    u8 _3c[0x48 - 0x3c];
};
size_assert(SceneFactory, 0x48);
