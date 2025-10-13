#pragma once

#include "game/BattleName2D.hh"
#include "game/Scene.hh"
#include "game/SceneType.hh"

#include <payload/Replace.hh>

class SceneFactory {
public:
    class ArchiveType {
    public:
        enum {
            Title = 0x0,
            Menu = 0x1,
            Option = 0x2,
            CourseSelect = 0x3,
            MapSelect = 0x4,
            GhostData = 0x6,
            LanEntry = 0x7,
            TitleLine = 0x8,
            Background = 0x9,
            BattleName = 0xb,
        };

    private:
        ArchiveType();
    };

    SceneFactory();
    ~SceneFactory();

    JKRArchive *archive(u32 archiveType);

    const char *string(u32 archiveType, u32 index);

    void REPLACED(loadData)(s32 sceneType, JKRHeap *heap);
    REPLACE void loadData(s32 sceneType, JKRHeap *heap);
    Scene *REPLACED(createScene)(s32 sceneType, JKRHeap *heap);
    REPLACE Scene *createScene(s32 sceneType, JKRHeap *heap);

    static SceneFactory *Create();
    static void Destroy();
    static SceneFactory *Instance();

private:
    void loadLocalizedArchive(u32 type, const char *name, JKRHeap *heap);

    JKRArchive *m_archives[0xf];
    u8 _3c[0x40 - 0x3c];
    BattleName2D *m_battleName2D;
    u8 _44[0x48 - 0x44];

    static SceneFactory *s_instance;
};
size_assert(SceneFactory, 0x48);
