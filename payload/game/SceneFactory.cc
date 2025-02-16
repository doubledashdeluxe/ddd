#include "SceneFactory.hh"

#include "game/ResMgr.hh"
#include "game/SceneCharacterSelect.hh"
#include "game/SceneCoursePoll.hh"
#include "game/SceneFormatSelect.hh"
#include "game/SceneHowManyPlayers.hh"
#include "game/SceneMapSelect.hh"
#include "game/SceneModeSelect.hh"
#include "game/SceneNameSelect.hh"
#include "game/SceneOption.hh"
#include "game/ScenePackSelect.hh"
#include "game/ScenePlayerList.hh"
#include "game/SceneRoomCodeEnter.hh"
#include "game/SceneRoomTypeSelect.hh"
#include "game/SceneServerSelect.hh"
#include "game/SceneTandemSelect.hh"
#include "game/SceneTitle.hh"
#include "game/SysDebug.hh"

JKRArchive *SceneFactory::archive(u32 archiveType) {
    return m_archives[archiveType];
}

void SceneFactory::loadData(s32 sceneType, JKRHeap *heap) {
    ResMgr::SwapForMenu(heap);

    switch (sceneType) {
    case SceneType::Option:
        REPLACED(loadData)(SceneType::Option, heap);
        loadLocalizedArchive(ArchiveType::GhostData, "GhostData", heap);
        return;
    case SceneType::PackSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::PackSelect, heap);
        loadLocalizedArchive(ArchiveType::GhostData, "GhostData", heap);
        return;
    case SceneType::HowManyPlayers:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::LanEntry, heap);
        return;
    case SceneType::NameSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::LanEntry, heap);
        return;
    case SceneType::TandemSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::LanEntry, heap);
        return;
    case SceneType::ServerSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::LanEntry, heap);
        loadLocalizedArchive(ArchiveType::GhostData, "GhostData", heap);
        return;
    case SceneType::RoomTypeSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::LanEntry, heap);
        return;
    case SceneType::RoomCodeEnter:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::PackSelect, heap);
        REPLACED(loadData)(SceneType::LanEntry, heap);
        return;
    case SceneType::ModeSelect:
        loadLocalizedArchive(ArchiveType::GhostData, "GhostData", heap);
        return;
    case SceneType::FormatSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        return;
    case SceneType::PlayerList:
        REPLACED(loadData)(SceneType::LanEntry, heap);
        return;
    case SceneType::CharacterSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        return;
    case SceneType::MapSelect:
        REPLACED(loadData)(SceneType::Menu, heap);
        REPLACED(loadData)(SceneType::PackSelect, heap);
        loadLocalizedArchive(ArchiveType::CourseSelect, "CourseSelect", heap);
        loadLocalizedArchive(ArchiveType::GhostData, "GhostData", heap);
        return;
    case SceneType::CoursePoll:
        REPLACED(loadData)(SceneType::LanEntry, heap);
        loadLocalizedArchive(ArchiveType::CourseSelect, "CourseSelect", heap);
        return;
    }

    REPLACED(loadData)(sceneType, heap);
}

Scene *SceneFactory::createScene(s32 sceneType, JKRHeap *heap) {
    SysDebug *sysDebug = SysDebug::GetManager();
    Scene *scene;
    switch (sceneType) {
    case SceneType::Title:
        sysDebug->setHeapGroup("Title", heap);
        scene = new (heap, 0x0) SceneTitle(m_archives[ArchiveType::Title], heap);
        break;
    case SceneType::Option:
        sysDebug->setHeapGroup("Option", heap);
        scene = new (heap, 0x0) SceneOption(m_archives[ArchiveType::Option], heap);
        break;
    case SceneType::PackSelect:
        sysDebug->setHeapGroup("PackSelect", heap);
        scene = new (heap, 0x0) ScenePackSelect(m_archives[ArchiveType::GhostData], heap);
        break;
    case SceneType::HowManyPlayers:
        sysDebug->setHeapGroup("HowManyPlayers", heap);
        scene = new (heap, 0x0) SceneHowManyPlayers(m_archives[ArchiveType::Menu], heap);
        break;
    case SceneType::NameSelect:
        sysDebug->setHeapGroup("NameSelect", heap);
        scene = new (heap, 0x0) SceneNameSelect(m_archives[ArchiveType::Menu], heap);
        break;
    case SceneType::TandemSelect:
        sysDebug->setHeapGroup("TandemSelect", heap);
        scene = new (heap, 0x0) SceneTandemSelect(m_archives[ArchiveType::Menu], heap);
        break;
    case SceneType::ServerSelect:
        sysDebug->setHeapGroup("ServerSelect", heap);
        scene = new (heap, 0x0) SceneServerSelect(m_archives[ArchiveType::GhostData], heap);
        break;
    case SceneType::RoomTypeSelect:
        sysDebug->setHeapGroup("RoomTypeSelect", heap);
        scene = new (heap, 0x0) SceneRoomTypeSelect(m_archives[ArchiveType::Menu], heap);
        break;
    case SceneType::RoomCodeEnter:
        sysDebug->setHeapGroup("RoomCodeEnter", heap);
        scene = new (heap, 0x0) SceneRoomCodeEnter(m_archives[ArchiveType::LanEntry], heap);
        break;
    case SceneType::ModeSelect:
        sysDebug->setHeapGroup("ModeSelect", heap);
        scene = new (heap, 0x0) SceneModeSelect(m_archives[ArchiveType::GhostData], heap);
        break;
    case SceneType::FormatSelect:
        sysDebug->setHeapGroup("FormatSelect", heap);
        scene = new (heap, 0x0) SceneFormatSelect(m_archives[ArchiveType::Menu], heap);
        break;
    case SceneType::PlayerList:
        sysDebug->setHeapGroup("PlayerList", heap);
        scene = new (heap, 0x0) ScenePlayerList(m_archives[ArchiveType::LanEntry], heap);
        break;
    case SceneType::CharacterSelect:
        sysDebug->setHeapGroup("CharacterSelect", heap);
        scene = new (heap, 0x0) SceneCharacterSelect(m_archives[ArchiveType::Menu], heap);
        break;
    case SceneType::MapSelect:
        sysDebug->setHeapGroup("MapSelect", heap);
        m_battleName2D = BattleName2D::Create(m_archives[ArchiveType::BattleName]);
        scene = new (heap, 0x0) SceneMapSelect(m_archives[ArchiveType::MapSelect], heap);
        break;
    case SceneType::CoursePoll:
        sysDebug->setHeapGroup("CoursePoll", heap);
        scene = new (heap, 0x0) SceneCoursePoll(m_archives[ArchiveType::MapSelect], heap);
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

void SceneFactory::loadLocalizedArchive(u32 type, const char *name, JKRHeap *heap) {
    if (m_archives[type]) {
        return;
    }

    const char *languageName = KartLocale::GetLanguageName();
    Array<char, 64> path;
    snprintf(path.values(), path.count(), "/SceneData/%s/%s.arc", languageName, name);
    m_archives[type] = JKRArchive::Mount(path.values(), JKRArchive::MountMode::Mem, heap,
            JKRArchive::MountDirection::Head);
}
