#pragma once

#include "game/GameApp.hh"
#include "game/Scene.hh"
#include "game/SceneType.hh"

#include <common/Array.hh>
#include <payload/Replace.hh>

class SequenceApp : public GameApp {
public:
    bool ready(s32 sceneType);
    s32 prevScene() const;
    void setNextScene(s32 sceneType);

    static void REPLACED(Call)(s32 sceneType);
    REPLACE static void Call(s32 sceneType);
    REPLACE static SequenceApp *Create();
    static SequenceApp *Instance();

private:
    SequenceApp();
    ~SequenceApp() override;
    void draw() override;
    void calc() override;
    void reset() override;

    REPLACE void freeForMovieApp();
    REPLACE Scene *createScene(s32 sceneType);

    Scene *m_scene;
    u8 _10[0x38 - 0x10]; // Unused
    u32 m_loadingFlag;
    u32 m_loadFlag;
    u32 m_prevScene;
    u32 m_state;
    u32 _48;
    Array<Scene *, SceneType::Max> m_scenes; // Added

    static SequenceApp *s_instance;
    static u32 s_scene;
    static u32 s_nextScene;
};
size_assert(SequenceApp, 0x4c + sizeof(Array<Scene *, SceneType::Max>));
