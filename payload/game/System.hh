#pragma once

#include <jsystem/JFWDisplay.hh>
#include <jsystem/JKRHeap.hh>
#include <jsystem/JKRTask.hh>
#include <payload/Replace.hh>

class System {
public:
    static void REPLACED(Init)();
    REPLACE static void Init();
    static JFWDisplay *GetDisplay();
    static JKRHeap *GetAppHeap();

private:
    System();

    static void REPLACED(StartAudio)(void *userData);
    REPLACE static void StartAudio(void *userData);

    static JFWDisplay *s_display;
    static JKRHeap *s_appHeap;
    static JKRTask *s_loadTask;
};
size_assert(System, 0x1);
