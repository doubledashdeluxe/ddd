#include "SceneFactory.hh"

#include "game/SceneMapSelect.hh"
#include "game/SysDebug.hh"

Scene *SceneFactory::createScene(s32 sceneType, JKRHeap *heap) {
    SysDebug *sysDebug = SysDebug::GetManager();
    Scene *scene;
    switch (sceneType) {
    case SceneType::MapSelect:
        sysDebug->setHeapGroup("MapSelect", heap);
        scene = new (heap, 0x0) SceneMapSelect(m_archives[sceneType], heap);
        break;
    default:
        return REPLACED(createScene)(sceneType, heap);
    }
    sysDebug->setDefaultHeapGroup(heap);
    return scene;
}
