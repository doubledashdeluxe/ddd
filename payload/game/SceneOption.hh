#pragma once

#include "game/Scene.hh"
#include "game/SceneTitleLine.hh"

#include <portable/Array.hh>

class SceneOption : public Scene {
public:
    SceneOption(JKRArchive *archive, JKRHeap *heap);
    ~SceneOption() override;
    void init() override;
    void draw() override;
    void calc() override;

private:
    class Option {
    public:
        enum {
            Sound = 0,
            Volume = 1,
            Rumble = 2,
            Ghosts = 3,
            Items = 4,
            Laps = 5,
            P1RearView = 6,
            P2RearView = 7,
            P3RearView = 8,
            P4RearView = 9,
            AspectRatio = 10,
            Count = 11,
        };

    private:
        Option();
    };

    class Entry {
    public:
        enum {
            Count = Option::Count + 1,
        };

    private:
        Entry();
    };

    typedef void (SceneOption::*State)();

    void slideIn();
    void slideOut();
    void idle();
    void scrollUp();
    void scrollDown();
    void wait();
    void nextScene();

    void stateSlideIn();
    void stateSlideOut();
    void stateIdle();
    void stateScrollUp();
    void stateScrollDown();
    void stateWait();
    void stateNextScene();

    void readOptions();
    void writeOptions();
    void onOptionChange();
    void refreshOption(u32 index, const char **valueNames);
    void refreshOption(u32 index, const char *valueNames);
    void refreshOption(u32 index, s8 nameOffset);
    void refreshOption(const char *tag, const char *name);
    void refreshOption(const char *tag, char name);
    void refreshOption(const char *tag, u8 name);
    void showOptions();
    void showArrows();
    void hideArrows();

    State m_state;
    u32 m_entryIndex;
    u32 m_rowIndex;
    Array<u8, Option::Count> m_savedValues;
    Array<u8, Option::Count> m_unsavedValues;
    u8 m_timeBeforeRumbleEnd;
    J2DScreen m_backgroundScreen;
    J2DScreen m_mainScreen;
    J2DScreen m_arrowScreen;
    J2DAnmBase *m_backgroundAnmTextureSRTKey;
    J2DAnmBase *m_mainAnmTransform;
    J2DAnmBase *m_scrollAnmTransform;
    J2DAnmBase *m_cursorAnmTransform;
    Array<J2DAnmBase *, Entry::Count> m_entryAnmTransforms;
    Array<J2DAnmBase *, Option::Count> m_optionAnmTransforms;
    Array<J2DAnmBase *, Option::Count> m_optionLeftAnmTransforms;
    Array<J2DAnmBase *, Option::Count> m_optionLeftAnmTevRegKeys;
    Array<J2DAnmBase *, Option::Count> m_optionRightAnmTransforms;
    Array<J2DAnmBase *, Option::Count> m_optionRightAnmTevRegKeys;
    Array<J2DAnmBase *, Option::Count> m_optionLoopAnmTransforms;
    Array<J2DAnmBase *, Option::Count> m_optionLoopAnmTextureSRTKeys;
    J2DAnmBase *m_volumeAnmTransform;
    J2DAnmBase *m_volumeAnmTextureSRTKey;
    J2DAnmBase *m_arrowAnmTransform;
    u16 m_backgroundAnmTextureSRTKeyFrame;
    u8 m_mainAnmTransformFrame;
    u8 m_scrollAnmTransformFrame;
    u8 m_cursorAnmTransformFrame;
    Array<u8, Entry::Count> m_entryAnmTransformFrames;
    Array<u8, Entry::Count> m_optionAnmTransformFrames;
    Array<u8, Entry::Count> m_optionLeftAnmTransformFrames;
    Array<u8, Entry::Count> m_optionLeftAnmTevRegKeyFrames;
    Array<u8, Entry::Count> m_optionRightAnmTransformFrames;
    Array<u8, Entry::Count> m_optionRightAnmTevRegKeyFrames;
    Array<u8, Entry::Count> m_optionLoopAnmTransformFrames;
    Array<u8, Entry::Count> m_optionLoopAnmTextureSRTKeyFrames;
    f32 m_volumeAnmTransformFrame;
    f32 m_volumeAnmTextureSRTKeyFrame;
    u8 m_arrowAnmTransformFrame;
    SceneTitleLine m_titleLine;
    s32 m_titleLineDirection;
    Array<u8, Option::Count> m_optionAlphas;
    Array<u8, 2> m_arrowAlphas;

    static const u32 ValueCounts[Option::Count];
    static const char *const EntryNames[Entry::Count];
};
