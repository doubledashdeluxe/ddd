#include "SceneFactory.hh"

#include "game/SceneMapSelect.hh"
#include "game/SceneOption.hh"
#include "game/ScenePackSelect.hh"
#include "game/SysDebug.hh"

JKRArchive *SceneFactory::archive(u32 archiveType) {
    return m_archives[archiveType];
}

void SceneFactory::loadData(s32 sceneType, JKRHeap *heap) {
    switch (sceneType) {
    case SceneType::Title:
        REPLACED(loadData)(SceneType::Title, heap);
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::Option, heap);
        REPLACED(loadData)(SceneType::Record, heap);
        REPLACED(loadData)(SceneType::GhostLoadSave, heap);
        return;
    case SceneType::Option:
        REPLACED(loadData)(SceneType::Option, heap);
        REPLACED(loadData)(SceneType::GhostLoadSave, heap);
        return;
    case SceneType::PackSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::PackSelect, heap);
        REPLACED(loadData)(SceneType::GhostLoadSave, heap);
        return;
    case SceneType::MapSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::CourseSelect, heap);
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
    case SceneType::Option:
        sysDebug->setHeapGroup("Option", heap);
        scene = new (heap, 0x0) SceneOption(m_archives[ArchiveType::Option], heap);
        break;
    case SceneType::PackSelect:
        sysDebug->setHeapGroup("PackSelect", heap);
        scene = new (heap, 0x0) ScenePackSelect(m_archives[ArchiveType::Menu], heap);
        break;
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
