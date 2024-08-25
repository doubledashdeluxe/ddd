#pragma once

#include <jsystem/J2DOrthoGraph.hh>
#include <jsystem/JFWDisplay.hh>
#include <jsystem/JKRHeap.hh>
#include <jsystem/JKRTask.hh>
#include <payload/Replace.hh>

class System {
public:
    class AspectRatio {
    public:
        enum {
            Count = 8,
        };

    private:
        AspectRatio();
    };

    static void REPLACED(Init)();
    REPLACE static void Init();
    REPLACE static void Run();

    static JFWDisplay *GetDisplay();
    static JKRHeap *GetAppHeap();
    static JKRTask *GetLoadTask();
    static f32 Get3DVpX();
    static f32 Get3DVpW();
    static f32 Get3DVpDiv2X(u8 index);
    static f32 Get3DVpDiv2W(u8 index);
    static f32 Get3DVpDiv4X(u8 index);
    static f32 Get3DVpDiv4W(u8 index);

    static void SetAspectRatio(u32 index);

private:
    System();

    static void REPLACED(StartAudio)(void *userData);
    REPLACE static void StartAudio(void *userData);

    static JFWDisplay *s_display;
    static JKRHeap *s_appHeap;
    static s32 s_dvdState;
    static JKRTask *s_loadTask;
    static J2DOrthoGraph *s_j2dOrtho;
    static f32 s_aspect;
    static f32 s_aspectDiv2;
    static f32 s_aspectSub;
    static f32 s_defaultAspectRatio;
};
size_assert(System, 0x1);
