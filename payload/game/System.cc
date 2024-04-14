#include "System.hh"

#include <common/SC.hh>
#include <jsystem/J2DPane.hh>

void System::Init() {
    REPLACED(Init)();
    s_loadTask->request(REPLACED(StartAudio), nullptr, nullptr);

    SC sc;
    u8 ar;
    if (sc.get("IPL.AR", ar) && ar) {
        s_defaultAspectRatio = 38.0f / 21.0f;
    }
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

void System::SetAspectRatio(u32 index) {
    f32 aspectRatios[AspectRatio::Count];
    aspectRatios[0] = s_defaultAspectRatio;
    aspectRatios[1] = 4.0f / 3.0f;
    aspectRatios[2] = 19.0f / 14.0f;
    aspectRatios[3] = 3.0f / 2.0f;
    aspectRatios[4] = 16.0f / 10.0f;
    aspectRatios[5] = 16.0f / 9.0f;
    aspectRatios[6] = 38.0f / 21.0f;
    aspectRatios[7] = 21.0f / 9.0f;
    f32 aspectRatio = aspectRatios[index];

    u32 width = aspectRatio / (19.0f / 14.0f) * 608.0f;
    s32 left = 0 + (608 - static_cast<s32>(width)) / 2;
    s32 right = 608 + (static_cast<s32>(width) - 608) / 2;
    s_j2dOrtho->m_orthoBox.start.x = left;
    s_j2dOrtho->m_orthoBox.end.x = right;

    s_aspect = aspectRatio;
    s_aspectDiv2 = aspectRatio * 448.0f / (0.5f * 448.0f - 1.0f);
    s_aspectSub = aspectRatio * 149.0f / 112.0f;

    J2DPane::SetAspectRatio(aspectRatio);
}

void System::StartAudio(void * /* userData */) {}

f32 System::s_defaultAspectRatio = 19.0f / 14.0f;
