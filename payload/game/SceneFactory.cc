#include "SceneFactory.hh"

#include "game/SceneMapSelect.hh"
#include "game/SysDebug.hh"

JKRArchive *SceneFactory::archive(u32 archiveType) {
    return m_archives[archiveType];
}

void SceneFactory::loadData(s32 sceneType, JKRHeap *heap) {
    REPLACED(loadData)(sceneType, heap);

    switch (sceneType) {
    case SceneType::MapSelect:
        REPLACED(loadData)(SceneType::GhostLoadSave, heap);
        break;
    }
}

Scene *SceneFactory::createScene(s32 sceneType, JKRHeap *heap) {
    SysDebug *sysDebug = SysDebug::GetManager();
    Scene *scene;
    switch (sceneType) {
    case SceneType::MapSelect:
        sysDebug->setHeapGroup("MapSelect", heap);
        m_battleName2D = BattleName2D::Create(m_archives[ArchiveType::BattleName]);
        scene = new (heap, 0x0) SceneMapSelect(m_archives[ArchiveType::MapSelect], heap);
        break;
    default:
        return REPLACED(createScene)(sceneType, heap);
    }
    sysDebug->setDefaultHeapGroup(heap);
    return scene;
}

SceneFactory *SceneFactory::Create() {
    s_instance = new SceneFactory;
    return s_instance;
}

void SceneFactory::Destroy() {
    delete s_instance;
    s_instance = nullptr;
}

SceneFactory *SceneFactory::Instance() {
    return s_instance;
}
