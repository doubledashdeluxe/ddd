#include "SceneFactory.hh"

Scene *SceneFactory::createScene(s32 sceneType, JKRHeap *heap) {
    switch (sceneType) {
    default:
        return REPLACED(createScene)(sceneType, heap);
    }
}
