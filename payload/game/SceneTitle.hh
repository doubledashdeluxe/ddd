#pragma once

#include "game/PrintMemoryCard.hh"
#include "game/Scene.hh"

#include <jsystem/J2DScreen.hh>
#include <portable/Array.hh>

class SceneTitle : public Scene {
public:
    SceneTitle(JKRArchive *archive, JKRHeap *heap);
    ~SceneTitle() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    class Entry {
    public:
        enum {
            Local = 0,
            Records = 1,
            Options = 2,
            Remote = 3,
            Replays = 4,
            About = 5,
            Count = 6,
        };

    private:
        Entry();
    };

    typedef void (SceneTitle::*State)();

    void fadeIn();
    void fadeOut();
    void idle();
    void setupCard();
    void start();
    void startLAN();
    void nextScene();
    void nextRace();
    void nextMovie();

    void stateFadeIn();
    void stateFadeOut();
    void stateIdle();
    void stateSetupCard();
    void stateStart();
    void stateStartLAN();
    void stateNextScene();
    void stateNextRace();
    void stateNextMovie();

    static u32 NextScene(u32 entryIndex);
    static u32 SoundID(u32 entryIndex);

    State m_state;
    u32 m_timeBeforeDemo;
    u32 m_entryIndex;
    bool m_lanIsEnabled;
    u32 m_nextScene;
    PrintMemoryCard m_printMemoryCard;
    J2DScreen m_screen;
    J2DAnmBase *m_startAnmTevRegKey;
    Array<J2DAnmBase *, Entry::Count> m_entryAnmTransforms;
    Array<J2DAnmBase *, Entry::Count> m_entryAnmTevRegKeys;
    Array<J2DAnmBase *, Entry::Count> m_entryLineAnmTransforms;
    Array<J2DAnmBase *, Entry::Count> m_entryLineAnmTevRegKeys;
    u8 m_startAnmTevRegKeyFrame;
    Array<u8, Entry::Count> m_entryAnmTransformFrames;
    Array<u8, Entry::Count> m_entryAnmTevRegKeyFrames;
    Array<u8, Entry::Count> m_entryLineAnmTransformFrames;
    Array<u8, Entry::Count> m_entryLineAnmTevRegKeyFrames;

    static u32 s_demoType;
};
