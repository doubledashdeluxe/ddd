#pragma once

#include <jsystem/JKRTask.hh>
#include <payload/Replace.hh>

class System {
public:
    static void REPLACED(Init)();
    REPLACE static void Init();

private:
    System();

    static void REPLACED(StartAudio)(void *userData);
    REPLACE static void StartAudio(void *userData);

    static JKRTask *s_loadTask;
};
static_assert(sizeof(System) == 0x1);
