#include "GameApp.hh"

#include "game/AwardApp.hh"
#include "game/SysDebug.hh"
#include "game/System.hh"

#include <jsystem/JKRAssertHeap.hh>
#include <jsystem/JKRExpHeap.hh>

extern "C" {
#include <string.h>
}

GameApp::GameApp(u32 size, const char *name, void *ptr) : m_ptr(ptr) {
    // Hack: this should be done in the RaceApp constructor, but it's just way too long
    AwardApp *awardApp = AwardApp::Instance();
    if (!strcmp(name, "Race") && awardApp) {
        m_heap = awardApp->m_heap;
        awardApp->m_heap = JKRAssertHeap::Create(System::GetAppHeap());
    } else if (ptr) {
        m_heap = JKRExpHeap::Create(ptr, size, nullptr, false);
    } else if (size) {
        m_heap = JKRExpHeap::Create(size, System::GetAppHeap(), false);
    } else {
        m_heap = JKRAssertHeap::Create(System::GetAppHeap());
    }
    m_heap->becomeCurrentHeap();
    SysDebug::GetManager()->createHeapInfo(m_heap, name);
}
