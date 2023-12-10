#include "SceneFactory.hh"

#include "game/SceneMapSelect.hh"
#include "game/ScenePackSelect.hh"
#include "game/SysDebug.hh"

JKRArchive *SceneFactory::archive(u32 archiveType) {
    return m_archives[archiveType];
}

void SceneFactory::loadData(s32 sceneType, JKRHeap *heap) {
    switch (sceneType) {
    case SceneType::MapSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::CourseSelect, heap);
        REPLACED(loadData)(SceneType::PackSelect, heap);
        REPLACED(loadData)(SceneType::GhostLoadSave, heap);
        return;
    case SceneType::PackSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::PackSelect, heap);
        REPLACED(loadData)(SceneType::GhostLoadSave, heap);
        return;
    }

    REPLACED(loadData)(sceneType, heap);
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
    case SceneType::PackSelect:
        sysDebug->setHeapGroup("PackSelect", heap);
        scene = new (heap, 0x0) ScenePackSelect(m_archives[ArchiveType::Menu], heap);
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
