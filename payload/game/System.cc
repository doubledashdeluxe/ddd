#include "System.hh"

void System::Init() {
    REPLACED(Init)();
    s_loadTask->request(REPLACED(StartAudio), nullptr, nullptr);
}

JFWDisplay *System::GetDisplay() {
    return s_display;
}

JKRHeap *System::GetAppHeap() {
    return s_appHeap;
}

JKRTask *System::GetLoadTask() {
    return s_loadTask;
}

void System::StartAudio(void * /* userData */) {}
